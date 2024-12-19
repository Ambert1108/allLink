/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

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
  Controller::Controller(SignlingInteractionSystem* client, VisionCnetralBase* vcb, JanusInteractionSystem* janus)
  : client_(client), vision_(vcb), janus_(janus), janusEngine(nullptr) {
    client_->registerObserver(this);
    vision_->registerObserver(this);
    //janus_->registerObserver(this);
    // 使用连接引擎
    int callType = seeker::IniConfig::GetInteger("this", "call_type", 0);
    if (callType == 1) {
      ServerInfo janusServerInfo(seeker::IniConfig::Get("this", "janus", "10.1.29.246:8188"));
      janusEngine = std::make_shared<rtcengine::Janitor>(janusServerInfo.serverIp_, janusServerInfo.serverPort_);
      janusEngine->registerObserver(this);
    }
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
    D_LOG("init PeerConnection");
    AddTracks();
    I_LOG("Current audio input device:");
    audioEngine.GetRecordingDevices(audioInputDevMap);
    audioEngine.setMicrophoneVolume(50);
    int videoType = seeker::IniConfig::GetInteger("this", "video_type", 0);
    if(videoType == 0) videoEngine.switchCamera(false);
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

  void Controller::DeletePeerConnection() {
    vision_->stopLocalRenderer();
    vision_->stopRemoteRenderer();
    videoEngine.close();
    audioEngine.close();
    peerConnection_ = nullptr;
    peerConnectionFactory_ = nullptr;
    meetId_.clear();
    // 重新注册nosip插件
    janusEngine->addNoSIP();
  }

  void Controller::EnsureStreamingUI() {

  }

  void Controller::AddTracks() {
    if (!peerConnection_->GetSenders().empty()) {
      return;  // 轨道已添加
    }

    // 创建音频轨道
    //rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
    //  peerConnectionFactory_->CreateAudioTrack(
    //    "audio_label",
    //    peerConnectionFactory_->CreateAudioSource(cricket::AudioOptions())
    //    .get()));
    //// 添加音频轨道到peerConnection
    //auto result_or_error = peerConnection_->AddTrack(audio_track, { "stream_id" });
    //if (!result_or_error.ok()) {
    //  E_LOG("Failed to add audio track to PeerConnection:{}", result_or_error.error().message());
    //}
    audioEngine.AddAudioTracks(peerConnectionFactory_, peerConnection_);

    // 寻找本地采集设备
    //rtc::scoped_refptr<CapturerTrackSource> video_device = CapturerTrackSource::Create();
    //if (video_device) {
    //  // 创建视频轨道
    //  rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_(
    //    peerConnectionFactory_->CreateVideoTrack(video_device, "video_label"));
    //
    //  // 向视觉控制器添加本地渲染器
    //  vision_->startLocalRenderer(video_track_.get());
    //
    //  // 添加视频轨道到peerConnection
    //  auto result_or_error = peerConnection_->AddTrack(video_track_, { "stream_id" });
    //  if (!result_or_error.ok()) {
    //    E_LOG("Failed to add video track to PeerConnection: {}", result_or_error.error().message());
    //  }
    //}
    //else {
    //  E_LOG("OpenVideoCaptureDevice failed");
    //}
    rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_;
    I_LOG("1");
    int videoType = seeker::IniConfig::GetInteger("this", "video_type", 0);
    I_LOG("2");
    if(videoType == 1) videoEngine.addScreenTrack(peerConnectionFactory_, peerConnection_, video_track_);
    else videoEngine.addVideoTrack(peerConnectionFactory_, peerConnection_, video_track_);
    //videoEngine.addVideoTrack(peerConnectionFactory_, peerConnection_, video_track_);
    I_LOG("3");
    // 向视觉控制器添加本地渲染器
    vision_->startLocalRenderer(video_track_.get());
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
    int callType = seeker::IniConfig::GetInteger("this", "call_type", 0);
    if (callType == 1 && 
      new_state == webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete) {
      W_LOG("[Controller::OnIceGatheringChange] ICE Candidate gather finish");
      hi::PostMsg({ msgTo(MessageType::SEND_ICE_COMPLETE_TO_PEER), nullptr });
    }
  }

  // 生成offer/answer后PeerConnectionObserver会通过此函数上传生成的candidate
  void Controller::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
    //if (loopback_) {
    //  if (!peerConnection_->AddIceCandidate(candidate)) {
    //    W_LOG("Failed to apply the received candidate");
    //  }
    //  return;
    //}

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

    Json::StreamWriterBuilder factory;
    std::string obj = (Json::writeString(factory, jmessage));
    hi::PostMsg({ msgTo(MessageType::SEND_ICE_TO_PEER), obj });
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
      if (type_str == "offer-loopback") {
        // This is a loopback call.
        // Recreate the peerconnection with DTLS disabled.
        //if (!ReinitializePeerConnectionForLoopback()) {
        //  E_LOG("Failed to initialize our PeerConnection instance");
        //  DeletePeerConnection();
        //  client_->SignOut();
        //}
        //D_LOG("on peer 6-2");
        return;
      }
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

  void Controller::OnCSMessageFromSignling(const SignInfo& info) {
    if (!peerConnection_.get()) {
      I_LOG("callee on message");
      meetId_ = info.from();
      // 如果没有peerConnection代表终端作为被叫
      Jsep sdp;
      sdp.sdp = info.sdp();
      sdp.type = "offer";
      hi::PostMsg({ msgTo(MessageType::SEND_PROCESS_TO_JANUS), sdp });
    }
    else {
      I_LOG("caller on message");
      if (info.from() != meetId_ && info.from() != "system") {
        // 在1v1流程中，判断主叫保存的呼叫id和信令发来的from是否一致
        // 如果是会议流程，信令发来的from应该是from
        E_LOG("[Controller::OnCSMessageFromSignling] meet id {} and from {} not match",
          meetId_, info.from());
        hi::PostMsg({ msgTo(MessageType::SEND_MSG_FAILED), nullptr });
        return;
      }
      Jsep sdp;
      sdp.sdp = info.sdp();
      sdp.type = "answer";
      hi::PostMsg({ msgTo(MessageType::SEND_PROCESS_TO_JANUS), sdp });
    }
  }

  //
  // JanusInteractionObserver implementation.
  //

  // Janus回复generate请求后需要告知信令并透传给对端
  /*void Controller::OnGenerated(const Jsep& tranditional) {
    hi::PostMsg({ msgTo(MessageType::SEND_SDP_TO_PEER), tranditional });
  }*/

  // Janus回复process请求后需要设置为远端会话描述以获取Janus的ICE候选
  //void Controller::OnProcessed(const Jsep& jsep) {
  //  std::string msg = seeker::json::toJsonString(jsep);
  //  SignInfo info;
  //  info.set_sdp(msg);
  //  // 此处设置，若程序作为主叫触发OnProcessed，meetId的值来源于ConnectToPeer函数中用户输入
  //  // 若程序作为被叫触发OnProcessed，meetId的值来源于OnCSMessageFromSignling函数中信令透传offer时的from值
  //  info.set_from(meetId_);
  //  //OnMessageFromSignling(info);
  //  hi::PostMsg({ msgTo(MessageType::SET_REMOTE_DESC), info });
  //}

  //
  // JanitorObserver implementation.
  //

  void Controller::OnGenerated(const std::string& sdp, const std::string& type) {
    I_LOG("on generated");
    Jsep tranditional;
    tranditional.sdp = sdp;
    tranditional.type = type;
    hi::PostMsg({ msgTo(MessageType::SEND_SDP_TO_PEER), tranditional });
  }

  void Controller::OnProcessed(const std::string& sdp, const std::string& type) {
    Jsep jsep;
    jsep.sdp = sdp;
    jsep.type = type;
    std::string msg = seeker::json::toJsonString(jsep);
    SignInfo info;
    info.set_sdp(msg);
    // 此处设置，若程序作为主叫触发OnProcessed，meetId的值来源于ConnectToPeer函数中用户输入
    // 若程序作为被叫触发OnProcessed，meetId的值来源于OnCSMessageFromSignling函数中信令透传offer时的from值
    info.set_from(meetId_);
    hi::PostMsg({ msgTo(MessageType::SET_REMOTE_DESC), info });
  }

  void Controller::OnReconnect() {
    W_LOG("[Controller::OnReconnect] Network disconnection detected, start reconnect");
    hi::PostMsg({ msgTo(MessageType::RECONNECT_PEER), nullptr });
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

  }


  bool Controller::ConnectToPeer(const std::string& to) {
    //用户触发
    if (peerConnection_.get()) {
      E_LOG("[Controller::ConnectToPeer] Only one call can be established at a time");
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
    int callType = seeker::IniConfig::GetInteger("this", "call_type", 0);
    if (callType == 0) {
      switch (msg.id) {
      case msgTo(MessageType::SEND_ICE_TO_PEER):
      case msgTo(MessageType::SEND_JSEP_SDP_TO_PEER): {
        // p2p流程，通知信令交互系统处理 offer/answer sdp 或 ice candidate
        if (!client_->sendToPeer(meetId_, std::any_cast<std::string>(msg.data))) {
          hi::PostMsg({ msgTo(MessageType::SEND_MSG_FAILED), nullptr });
        }
        break;
      }
      case msgTo(MessageType::DISCONNECT_PEER): {
        if (peerConnection_.get()) {
          DeletePeerConnection();
          I_LOG("delete callee peer connection");
        }
        break;
      }
      case msgTo(MessageType::RECONNECT_PEER): {
        break;
      }
      case msgTo(MessageType::LOGIN_SUCCESS): {
        I_LOG("[Controller::CustomMessageCallback] login success");
        int callType = seeker::IniConfig::GetInteger("this", "call_type", 0);
        if (callType == 1) {
          //ServerInfo janusServerInfo(seeker::IniConfig::Get("this", "janus", "10.1.29.246:8188"));
          //janus_->connectServer(janusServerInfo);
          janusEngine->init();
        }
        break;
      }
      default:
        break;
      }
    }
    else if (callType == 1) {
      // nosip流程
      switch (msg.id) {
      case msgTo(MessageType::LOGIN_SUCCESS): {
        I_LOG("[Controller::CustomMessageCallback] login success");
        int callType = seeker::IniConfig::GetInteger("this", "call_type", 0);
        if (callType == 1) {
          janusEngine->init();
        }
        break;
      }
      case msgTo(MessageType::MEETING_OK): {
        client_->sendAck(meetId_);
        break;
      }
      case msgTo(MessageType::SEND_SDP_TO_PEER): {
        Jsep jsep = std::any_cast<Jsep>(msg.data);
        if (!client_->sendToPeer(meetId_, jsep.sdp)) {
          hi::PostMsg({ msgTo(MessageType::SEND_MSG_FAILED), nullptr });
        }
        break;
      }
      case msgTo(MessageType::SEND_JSEP_SDP_TO_PEER): {
        //if (!janus_->sendGenerateToJanus(std::any_cast<std::string>(msg.data))) {
        //  hi::PostMsg({ msgTo(MessageType::SEND_MSG_FAILED), nullptr });
        //}
        Jsep jsep;
        seeker::json::fromJsonString(jsep, std::any_cast<std::string>(msg.data));
        janusEngine->generateSDP(jsep.sdp, jsep.type);
        break;
      }
      case msgTo(MessageType::SEND_PROCESS_TO_JANUS): {
        
        Jsep jsep = std::any_cast<Jsep>(msg.data);
        //if (!janus_->sendProcessToJanus(jsep.sdp, jsep.type)) {
        //  hi::PostMsg({ msgTo(MessageType::SEND_MSG_FAILED), nullptr });
        //}
        janusEngine->processSDP(jsep.sdp, jsep.type);
        break;
      }
      case msgTo(MessageType::SEND_ICE_TO_PEER): {
        //if (!janus_->sendTrckileToJanus(std::any_cast<std::string>(msg.data))) {
        //  hi::PostMsg({ msgTo(MessageType::SEND_MSG_FAILED), nullptr });
        //}
        janusEngine->sendTrickleToJanus(std::any_cast<std::string>(msg.data));
        break;
      }
      case msgTo(MessageType::SEND_ICE_COMPLETE_TO_PEER): {
        //if (!janus_->sendTrckileCompleteToJanus()) {
        //  hi::PostMsg({ msgTo(MessageType::SEND_MSG_FAILED), nullptr });
        //}
        janusEngine->sendTrickleCompleteToJanus();
        break;
      }
      case msgTo(MessageType::SET_REMOTE_DESC): {
        SignInfo info = std::any_cast<SignInfo>(msg.data);
        OnMessageFromSignling(info);
        break;
      }
      case msgTo(MessageType::SWITCH_AUDIO_INPUT): {
        int device = std::any_cast<int>(msg.data);
        auto it = audioInputDevMap.find(device);
        if (it != audioInputDevMap.end()) I_LOG("pick mic input device:{}", it->second);
        audioEngine.ReplaceRecordingDevices(device);
        break;
      }
      case msgTo(MessageType::SWITCH_MIC_VOLUME): {
        int volume = std::any_cast<int>(msg.data);
        I_LOG("current mic volume={}", volume);
        audioEngine.setMicrophoneVolume(volume);
        break;
      }
      case msgTo(MessageType::SET_MIC_PHONE): {
        bool state = std::any_cast<bool>(msg.data);
        I_LOG("set micphone state {}", state);
        //audioEngine.setMicrophone(state);
        if (state) client_->sendInfo(meetId_, 21);
        else client_->sendInfo(meetId_, 20);
        break;
      }
      case msgTo(MessageType::SET_CAMERA): {
        bool state = std::any_cast<bool>(msg.data);
        int videoType = seeker::IniConfig::GetInteger("this", "video_type", 0);
        if (videoType == 0) {
          I_LOG("set camera state {}", state);
          videoEngine.switchCamera(state);
        }
        break;
      }
      case msgTo(MessageType::DISCONNECT_PEER): {
        if (peerConnection_.get()) {
          DeletePeerConnection();
          I_LOG("delete callee peer connection");
        }
        break;
      }
      case msgTo(MessageType::RECONNECT_PEER): {
        if (!client_->reLogin()) break;
        if (peerConnection_.get()) {
          DeletePeerConnection();
        }
        break;
      }
      default:
        break;
      }
    }
    else {
      W_LOG("[Controller::CustomMessageCallback] undefine call type:{}", callType);
    }
  }

  // 
  // CreateSessionDescriptionObserver implementation
  //

  void Controller::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
    std::string sdp;
    desc->ToString(&sdp);
    std::string sdpTmp = audioEngine.modifySdp(sdp);

    I_LOG("LOG SDP\n{}", sdpTmp);
    // For loopback test. To save some connecting delay.
    //if (loopback_) {
    //  // Replace message type from "offer" to "answer"
    //  std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
    //    webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, sdp);
    //  peer_connection_->SetRemoteDescription(
    //    DummySetSessionDescriptionObserver::Create().get(),
    //    session_description.release());
    //  return;
    //}
    webrtc::SdpType type = desc->GetType();
    webrtc::SdpParseError error;
    std::unique_ptr<webrtc::SessionDescriptionInterface> tdesc 
      = webrtc::CreateSessionDescription(type, sdpTmp, &error);

    peerConnection_->SetLocalDescription(
      DummySetSessionDescriptionObserver::Create().get(), tdesc.release());

    Json::Value jmessage;
    jmessage["type"] = webrtc::SdpTypeToString(type);
    jmessage["sdp"] = sdpTmp;

    Json::StreamWriterBuilder factory;
    std::string obj = Json::writeString(factory, jmessage);
    // 在peerConnection线程中无法直接执行信令
    // 使用消息队列在主线程中处理
    hi::PostMsg({ msgTo(MessageType::SEND_JSEP_SDP_TO_PEER), obj });
    I_LOG("create sdp success");
  }

  void Controller::OnFailure(webrtc::RTCError error) {
    E_LOG("Create Offer failed, {}:{}", ToString(error.type()), error.message());
    hi::PostMsg({ msgTo(MessageType::SEND_MSG_FAILED), nullptr });
  }
}