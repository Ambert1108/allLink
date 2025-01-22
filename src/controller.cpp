#include "controller.h"

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "absl/memory/memory.h"
#include "api/audio/audio_device.h"
#include "api/audio/audio_mixer.h"
#include "api/audio/audio_processing.h"
#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/audio_options.h"
#include "api/create_peerconnection_factory.h"
#include "api/rtp_sender_interface.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_decoder_factory_template.h"
#include "api/video_codecs/video_decoder_factory_template_dav1d_adapter.h"
#include "api/video_codecs/video_decoder_factory_template_libvpx_vp8_adapter.h"
#include "api/video_codecs/video_decoder_factory_template_libvpx_vp9_adapter.h"
#include "api/video_codecs/video_decoder_factory_template_open_h264_adapter.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "api/video_codecs/video_encoder_factory_template.h"
#include "api/video_codecs/video_encoder_factory_template_libaom_av1_adapter.h"
#include "api/video_codecs/video_encoder_factory_template_libvpx_vp8_adapter.h"
#include "api/video_codecs/video_encoder_factory_template_libvpx_vp9_adapter.h"
#include "api/video_codecs/video_encoder_factory_template_open_h264_adapter.h"
#include "examples/peerconnection/client/defaults.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "p2p/base/port_allocator.h"
#include "pc/video_track_source.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "rtc_base/strings/json.h"
#include "test/vcm_capturer.h"

namespace alllink {

  std::string GetEnvVarOrDefault(const char* env_var_name,
    const char* default_value) {
    std::string value;
    const char* env_var = getenv(env_var_name);
    if (env_var)
      value = env_var;

    if (value.empty())
      value = default_value;

    return value;
  }

  std::string GetPeerConnectionString() {
    return GetEnvVarOrDefault("WEBRTC_CONNECT", "stun:stun.l.google.com:19302");
  }

  std::string GetDefaultServerName() {
    return GetEnvVarOrDefault("WEBRTC_SERVER", "localhost");
  }

  class DummySetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver {
  public:
    static rtc::scoped_refptr<DummySetSessionDescriptionObserver> Create() {
      return rtc::make_ref_counted<DummySetSessionDescriptionObserver>();
    }
    virtual void OnSuccess() { RTC_LOG(LS_INFO) << __FUNCTION__; }
    virtual void OnFailure(webrtc::RTCError error) {
      RTC_LOG(LS_INFO) << __FUNCTION__ << " " << ToString(error.type()) << ": "
        << error.message();
    }
  };

  class CapturerTrackSource : public webrtc::VideoTrackSource {
  public:
    static rtc::scoped_refptr<CapturerTrackSource> Create() {
      const size_t kWidth = 640;
      const size_t kHeight = 480;
      const size_t kFps = 30;
      std::unique_ptr<webrtc::test::VcmCapturer> capturer;
      std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
        webrtc::VideoCaptureFactory::CreateDeviceInfo());
      if (!info) {
        return nullptr;
      }
      int num_devices = info->NumberOfDevices();
      for (int i = 0; i < num_devices; ++i) {
        capturer = absl::WrapUnique(
          webrtc::test::VcmCapturer::Create(kWidth, kHeight, kFps, i));
        if (capturer) {
          return rtc::make_ref_counted<CapturerTrackSource>(std::move(capturer));
        }
      }

      return nullptr;
    }

  protected:
    explicit CapturerTrackSource(
      std::unique_ptr<webrtc::test::VcmCapturer> capturer)
      : VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {}

  private:
    rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
      return capturer_.get();
    }
    std::unique_ptr<webrtc::test::VcmCapturer> capturer_;
  };

}  // namespace

namespace alllink {
  Controller::Controller(SignlingInteractionSystem* client, VisionCnetralBase* vcb)
  : client_(client), vision_(vcb) {
    client_->registerObserver(this);
    vision_->registerObserver(this);
    rtc::LogMessage::ConfigureLogging("info info");
  }

  void Controller::Close() {
    DeletePeerConnection();
  }

  Controller::~Controller() {

  }

  bool Controller::InitializePeerConnection() {
    if (!signaling_thread_.get()) {
      signaling_thread_ = rtc::Thread::CreateWithSocketServer();
      signaling_thread_->Start();
    }
    
    peerConnectionFactory_ = webrtc::CreatePeerConnectionFactory(
      nullptr /* network_thread */, nullptr /* worker_thread */,
      signaling_thread_.get() /* signal thread */, audioEngine.InitAdm() /* rtc_audio_engine_adm */,
      //signaling_thread_.get() /* signal thread */, nullptr /* rtc_audio_engine_adm */,
      webrtc::CreateBuiltinAudioEncoderFactory(),
      webrtc::CreateBuiltinAudioDecoderFactory(),
      std::make_unique<webrtc::VideoEncoderFactoryTemplate<
      webrtc::OpenH264EncoderTemplateAdapter>>(),
      std::make_unique<webrtc::VideoDecoderFactoryTemplate<
      webrtc::OpenH264DecoderTemplateAdapter>>(),
      nullptr /* audio_mixer */, nullptr /* audio_processing */);

    if (!peerConnectionFactory_) {
      E_LOG("[Controller::InitializePeerConnection] Failed to initialize PeerConnectionFactory");
      DeletePeerConnection();
      return false;
    }

    if (!CreatePeerConnection()) {
      E_LOG("[Controller::InitializePeerConnection] CreatePeerConnection failed");
      DeletePeerConnection();
      return false;
    }
    I_LOG("init PeerConnection");
    AddTracks();
    I_LOG("Current audio input device:");
    videoEngine.getCameraMap(videoInputDevMap);
    videoEngine.getScreenMap(shareScreenMap);
    videoEngine.getWinMap(shareWindowMap);
    for (const auto& [id, name] : shareWindowMap) {
      I_LOG("window list {}:{}", id, name);
    }
    audioEngine.GetRecordingDevices(audioInputDevMap);
    audioEngine.GetPlayoutDevices(audioOutputDevMap);
    audioEngine.setMicrophoneVolume(50);
    videoEngine.switchCamera(false);
    videoEngine.switchScreen(false);
    I_LOG("init finish");

    return true;
  }

  bool Controller::CreatePeerConnection() {
    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = GetPeerConnectionString();
    config.servers.push_back(server);

    webrtc::PeerConnectionDependencies pc_dependencies(this);
    auto error_or_peer_connection =
      peerConnectionFactory_->CreatePeerConnectionOrError(
        config, std::move(pc_dependencies));
    if (error_or_peer_connection.ok()) {
      peerConnection_ = std::move(error_or_peer_connection.value());
    }
    return peerConnection_ != nullptr;
  }

  void Controller::DeletePeerConnection(bool clear) {
    vision_->stopLocalRenderer();
    vision_->stopRemoteRenderer();
    screenTrackInterface.release();
    videoEngine.close();
    audioEngine.close();
    peerConnection_ = nullptr;
    peerConnectionFactory_ = nullptr;
    if(clear) meetId_.clear();
  }

  void Controller::AddTracks() {
    if (!peerConnection_->GetSenders().empty()) {
      I_LOG("pc senders is not empty");
      return;  // 轨道已添加
    }
    rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_, screen_track_;
    videoEngine.addVideoTrack(peerConnectionFactory_, peerConnection_, video_track_);
    videoEngine.addScreenTrack(peerConnectionFactory_, peerConnection_, screen_track_);
    audioEngine.AddAudioTracks(peerConnectionFactory_, peerConnection_);
    // 向视觉控制器添加本地渲染器
    vision_->startLocalRenderer(video_track_.get());
    screenTrackInterface = screen_track_;
  }

  //
  // PeerConnectionObserver implementation.
  //
  void Controller::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
    const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) {
    hi::PostMsg({ msgTo(MessageType::ADD_TRACK), receiver->track().release() });
  }

  void Controller::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
    hi::PostMsg({ msgTo(MessageType::REMOVE_TRACK), receiver->track().release() });
  }

  // ICE候选收集完成后触发此回调函数
  void Controller::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
    if (new_state == webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete) {
      W_LOG("[Controller::OnIceGatheringChange] ICE Candidate gather finish");
      hi::PostMsg({ msgTo(MessageType::SEND_ICE_COMPLETE_TO_PEER), nullptr });
    }
  }

  // 生成offer/answer后PeerConnectionObserver会通过此函数上传生成的candidate
  void Controller::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
    Json::Value jmessage;
    jmessage["sdpMid"] = candidate->sdp_mid();
    jmessage["sdpMLineIndex"] = candidate->sdp_mline_index();
    std::string sdp;
    if (!candidate->ToString(&sdp)) {
      E_LOG("Failed to serialize candidate");
      hi::PostMsg({ msgTo(MessageType::SEND_MSG_FAILED), nullptr });
      return;
    }
    jmessage["candidate"] = sdp;

    Candidate ice;
    ice.sdpMid = candidate->sdp_mid();
    ice.sdpMLineIndex = candidate->sdp_mline_index();
    ice.candidate = sdp;

    Json::StreamWriterBuilder factory;
    std::string obj = (Json::writeString(factory, jmessage));
    hi::PostMsg({ msgTo(MessageType::SEND_ICE_TO_PEER), ice });
  }


  //
  // SignlingInteractionObserver implementation.
  //

  void Controller::OnPeerDisconnected(const std::string& id) {
    if (meetId_ != id) {
      W_LOG("[Controller::OnPeerDisconnected] link id {} does not match request id {}", meetId_, id);
      return;
    }
    hi::PostMsg({ msgTo(MessageType::DISCONNECT_PEER), nullptr });
  }

  // 信令收到对端forward请求后进入此回调函数获取offer sdp并生成answer sdp
  void Controller::OnMessageFromSignling(const SignInfo& info) {
    if (!peerConnection_.get()) {
      meetId_ = info.from();
      if (!InitializePeerConnection()) {
        E_LOG("Failed to initialize our PeerConnection instance");
        return;
      }
      D_LOG("init peerConnection finish");
    }
    else if (meetId_ != info.from()) {
      W_LOG("Received a message from unknown peer while already in a "
        "conversation with a different peer.");
      return;
    }
    Json::CharReaderBuilder factory;
    std::unique_ptr<Json::CharReader> reader =
      absl::WrapUnique(factory.newCharReader());
    Json::Value jmessage;
    if (!reader->parse(info.sdp().data(), info.sdp().data() + info.sdp().length(), &jmessage, nullptr)) {
      W_LOG("Received unknown message:{}", info.sdp());
      return;
    }
    std::string type_str;
    std::string json_object;

    rtc::GetStringFromJsonObject(jmessage, "type", &type_str);

    if (!type_str.empty()) { //type不为空代表收到sdp
      std::optional<webrtc::SdpType> type_maybe = webrtc::SdpTypeFromString(type_str);
      if (!type_maybe) {
        E_LOG("Unknown SDP type: {}", type_str);
        return;
      }
      webrtc::SdpType type = *type_maybe;
      std::string sdp;
      if (!rtc::GetStringFromJsonObject(jmessage, "sdp", &sdp)) {
        W_LOG("Can't parse received session description message.");
        return;
      }
      webrtc::SdpParseError error;
      std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
        webrtc::CreateSessionDescription(type, sdp, &error);
      if (!session_description) {
        W_LOG("Can't parse received session description message. "
          "SdpParseError was: {}", error.description);
        return;
      }
      I_LOG("set remote sdp:{}, type:{}", sdp, type_str);
      peerConnection_->SetRemoteDescription(
        DummySetSessionDescriptionObserver::Create().get(),
        session_description.release());
      I_LOG("peerConnection setRemoteDescription finish");
      if (type == webrtc::SdpType::kOffer) { //如果收到的是offer sdp则需要创建answer sdp，成功后回调OnSuccess
        peerConnection_->CreateAnswer(this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
        I_LOG("create answer done");
      }
    }
    else {  //type为空代表收到ice candidate
      std::string sdp_mid;
      int sdp_mlineindex = 0;
      std::string sdp;
      if (!rtc::GetStringFromJsonObject(jmessage, "sdpMid",
        &sdp_mid) ||
        !rtc::GetIntFromJsonObject(jmessage, "sdpMLineIndex",
          &sdp_mlineindex) ||
        !rtc::GetStringFromJsonObject(jmessage, "candidate", &sdp)) {
        W_LOG("Can't parse received message.");
        return;
      }
      webrtc::SdpParseError error;
      std::unique_ptr<webrtc::IceCandidateInterface> candidate(
        webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp, &error));
      if (!candidate.get()) {
        W_LOG("Can't parse received candidate message. SdpParseError was: {}", error.description);
        return;
      }
      // 添加并应用远端的ICE Candidate
      if (!peerConnection_->AddIceCandidate(candidate.get())) {
        W_LOG("Failed to apply the received candidate");
        return;
      }
      I_LOG("Received candidate :{}", info.sdp());
    }
  }

  void Controller::OnSignlingDisconnect() {
    W_LOG("[Controller::OnSignlingDisconnect] Signling disconnection detected, start reconnect");
    hi::PostMsg({ msgTo(MessageType::RECONNECT_SERVER), nullptr });
  }

  void Controller::OnRinging() {
    webrtc::SdpParseError error;
    std::unique_ptr<webrtc::SessionDescriptionInterface> tdesc
      = webrtc::CreateSessionDescription(type, sdpTmp, &error);

    peerConnection_->SetLocalDescription(
      DummySetSessionDescriptionObserver::Create().get(), tdesc.release());
    I_LOG("[Controller::CustomMessageCallback] peerConnection setLocalDescription finish");
  }

  void Controller::OnInfoSuccess() {
    if (needRequestIFrame) {
      I_LOG("[Controller::OnInfoSuccess] request screen IFrame");
      videoEngine.requestKeyFrame();
      needRequestIFrame = false;
    }
  }

  //
  // VisionCnetralCallback implementation.
  //

  bool Controller::StartLogin(const ServerInfo& server, const UserInfo& user) {
    //调用信令接口实现登录
    if (!client_->connectServer(server)) {
      W_LOG("[Controller::StartLogin] link server {}:{} failed", server.serverIp_, server.serverPort_);
      return false;
    }
    if (!client_->login(user)) {
      W_LOG("[Controller::StartLogin] {} login failed", user.id_);
      return false;
    }
    I_LOG("[Controller::StartLogin] login user:{} to {}:{} done",
      user.id_, server.serverIp_, server.serverPort_);
    return true;
  }


  void Controller::DisconnectFromServer() {
    client_->disConnectServer();
  }


  bool Controller::ConnectToPeer(const std::string& to) {
    //用户触发
    if (peerConnection_.get()) {
      E_LOG("[Controller::ConnectToPeer] Only one call can be established at a time");
      return false;
    }
    std::regex pattern("^\\d{3}-\\d{3}$");
    if (!std::regex_match(to, pattern)) {
      E_LOG("[Controller::ConnectToPeer] meetingId {} error, example 123-456", to);
      return false;
    }
    if (!InitializePeerConnection()) {
      E_LOG("[Controller::ConnectToPeer] init peer connection failed");
      return false;
    }
    meetId_ = to;
    // 创建offer sdp
    // 生成offer后会通过OnSuccess回调函数传回offer
    // 在OnSuccess函数中触发信令流程
    peerConnection_->CreateOffer(this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    I_LOG("create offer done");
    hi::PostMsg({ msgTo(MessageType::AUDIO_INPUT_DEV_INFO), audioInputDevMap });
    hi::PostMsg({ msgTo(MessageType::AUDIO_OUTPUT_DEV_INFO), audioOutputDevMap });
    hi::PostMsg({ msgTo(MessageType::VIDEO_DEV_INFO), videoInputDevMap });
    hi::PostMsg({ msgTo(MessageType::SHARE_SCREEN_INFO), shareScreenMap });
    hi::PostMsg({ msgTo(MessageType::SHARE_WINDOW_INFO), shareWindowMap });
    return true;
  }


  void Controller::DisconnectFromCurrentPeer() {
    if (peerConnection_.get()) {
      client_->sendBye(meetId_);
      DeletePeerConnection();
      I_LOG("delete caller peer connection");
    }
  }


  void Controller::CustomMessageCallback(const Message& msg) {
    switch (msg.id) {
    case msgTo(MessageType::LOGIN_SUCCESS): {
      I_LOG("[Controller::CustomMessageCallback] login success");
      break;
    }
    case msgTo(MessageType::MEETING_OK): {
      client_->sendAck(meetId_);
      break;
    }
    case msgTo(MessageType::SEND_SDP_TO_PEER): {
      std::string sdp = std::any_cast<std::string>(msg.data);
      if (!client_->sendToPeer(meetId_, sdp)) {
        hi::PostMsg({ msgTo(MessageType::SEND_MSG_FAILED), nullptr });
      }
      break;
    }
    case msgTo(MessageType::SEND_ICE_TO_PEER): {
      client_->sendTrickle(meetId_, std::any_cast<Candidate>(msg.data));
      break;
    }
    case msgTo(MessageType::SEND_ICE_COMPLETE_TO_PEER): {
      client_->sendTrickleComplete(meetId_); 
      break;
    }
    case msgTo(MessageType::SWITCH_AUDIO_INPUT): {
      int device = std::any_cast<int>(msg.data);
      auto it = audioInputDevMap.find(device);
      if (it != audioInputDevMap.end()) I_LOG("[Controller::CustomMessageCallback] pick mic input device:{}", it->second);
      audioEngine.ReplaceRecordingDevices(device);
      break;
    }
    case msgTo(MessageType::SWITCH_AUDIO_INPUT_STR): {
      std::string device = std::any_cast<std::string>(msg.data);
      for (const auto& [id, name] : audioInputDevMap) {
        if (device == name) {
          I_LOG("[Controller::CustomMessageCallback] pick mic input device:{}", name);
          audioEngine.ReplaceRecordingDevices(id);
          break;
        }
      }
      break;
    }
    case msgTo(MessageType::SWITCH_AUDIO_OUTPUT_STR): {
      std::string device = std::any_cast<std::string>(msg.data);
      for (const auto& [id, name] : audioOutputDevMap) {
        if (device == name) {
          I_LOG("[Controller::CustomMessageCallback] pick mic output device:{}", name);
          audioEngine.ReplacePlayoutDevices(id);
          break;
        }
      }
      break;
    }
    case msgTo(MessageType::SWITCH_VIDEO_INPUT): {
      std::string device = std::any_cast<std::string>(msg.data);
      for (const auto& [id, name] : videoInputDevMap) {
        if (device == name) {
          I_LOG("[Controller::CustomMessageCallback] pick mic output device:{}", name);
          videoEngine.setCamera(id);
          break;
        }
      }
      break;
    }
    case msgTo(MessageType::SWITCH_SHARE_SCREEN): {
      std::string deviceId = std::any_cast<std::string>(msg.data);
      for (const auto& [id, name] : shareScreenMap) {
        if (std::atoi(deviceId.c_str()) == id) {
          I_LOG("[Controller::CustomMessageCallback] pick share screen:{}", id);
          videoEngine.setScreenCapture(id);
          break;
        }
      }
      break;
    }
    case msgTo(MessageType::SWITCH_SHARE_WINDOW): {
      std::string label = std::any_cast<std::string>(msg.data);
      for (const auto& [id, name] : shareWindowMap) {
        size_t lastDashIndex = name.find_last_of('-');
        if (lastDashIndex != std::string::npos) {
          if (label == name.substr(lastDashIndex + 1)) {
            I_LOG("[Controller::CustomMessageCallback] pick share window {}:{}", id, name);
            videoEngine.setWindowCapture(id);
            break;
          }
        }
        else {
          if (label == name) {
            I_LOG("[Controller::CustomMessageCallback] pick share window {}:{}", id, name);
            videoEngine.setWindowCapture(id);
            break;
          }
        }
      }
      break;
    }
    case msgTo(MessageType::SWITCH_MIC_VOLUME): {
      int volume = std::any_cast<int>(msg.data);
      I_LOG("[Controller::CustomMessageCallback] current mic volume={}", volume);
      audioEngine.setMicrophoneVolume(volume);
      break;
    }
    case msgTo(MessageType::SET_MIC_PHONE): {
      bool state = std::any_cast<bool>(msg.data);
      I_LOG("[Controller::CustomMessageCallback] set micphone state {}", state);
      if (state) client_->sendInfo(meetId_, 21);
      else client_->sendInfo(meetId_, 20);
      break;
    }
    case msgTo(MessageType::SET_CAMERA): {
      bool state = std::any_cast<bool>(msg.data);
      videoEngine.switchCamera(state);
      if (state) videoEngine.setVideoBitrate(0.9);
      else videoEngine.setVideoBitrate(0.2);
      break;
    }
    case msgTo(MessageType::SET_SHARE): {
      bool state = std::any_cast<bool>(msg.data);
      if (state) {
        if (!client_->sendInfo(meetId_, 31)) {
          W_LOG("[Controller::CustomMessageCallback] Open Share failed");
        }
        videoEngine.switchScreen(true);
        videoEngine.setVideoBitrate(1.6);
        needRequestIFrame = true;
      }
      else {
        if (!client_->sendInfo(meetId_, 30)) {
          W_LOG("[Controller::CustomMessageCallback] Close Share failed");
        }
        videoEngine.switchScreen(false);
        videoEngine.setVideoBitrate(0.9);
        needRequestIFrame = false;
      }
      break;
    }
    case msgTo(MessageType::REQUEST_IFRAME): {
      videoEngine.requestKeyFrame();
      break;
    }
    case msgTo(MessageType::DISCONNECT_PEER): {
      if (peerConnection_.get()) {
        DeletePeerConnection();
        I_LOG("delete callee peer connection");
      }
      break;
    }
    case msgTo(MessageType::RECONNECT_SERVER): {
      if (!client_->reLogin()) {
        hi::PostMsg({ msgTo(MessageType::RECONNECT_SERVER_FAILED), nullptr });
      }
      break;
    }
    case msgTo(MessageType::RECONNECT_PEER): {
      if (peerConnection_.get()) {
        DeletePeerConnection(false);
        I_LOG("renegotiation peer connection");
      }
      if (!meetId_.empty()) ConnectToPeer(meetId_);
      break;
    }
    default:
      break;
    }
  }

  // 
  // CreateSessionDescriptionObserver implementation
  //

  void Controller::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
    std::string sdp;
    desc->ToString(&sdp);
    sdpTmp = audioEngine.modifySdp(sdp);
    I_LOG("LOG SDP\n{}", sdpTmp);
    type = desc->GetType();

    Json::Value jmessage;
    jmessage["type"] = webrtc::SdpTypeToString(type);
    jmessage["sdp"] = sdpTmp;

    Json::StreamWriterBuilder factory;
    std::string obj = Json::writeString(factory, jmessage);
    // 在peerConnection线程中无法直接执行信令
    // 使用消息队列在主线程中处理
    hi::PostMsg({ msgTo(MessageType::SEND_SDP_TO_PEER), sdpTmp });
    I_LOG("create sdp success");
  }

  void Controller::OnFailure(webrtc::RTCError error) {
    E_LOG("Create Offer failed, {}:{}", ToString(error.type()), error.message());
    hi::PostMsg({ msgTo(MessageType::SEND_MSG_FAILED), nullptr });
  }
}