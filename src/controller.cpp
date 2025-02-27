#include "controller.h"
#include <tuple>

namespace alllink {
  Controller::Controller(VisionCentralBase* vcb)
  : vision_(vcb) {
    vision_->registerObserver(this);
    rtc::LogMessage::ConfigureLogging("info info");
  }

  void Controller::Close() {

  }

  Controller::~Controller() {
  }

  //
  // RtcConnectEngine implementation.
  //

  void Controller::OnLoginSuccess(std::string userId) {
    hi::PostMsg({ msgTo(MessageType::LOGIN_SUCCESS), userId });
  }

  void Controller::OnLoginFailure() {
    W_LOG("[Controller::OnLoginFailure] login failed");
  }

  void Controller::OnLogoutSuccess() {

  }

  void Controller::OnReceiveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
    hi::PostMsg({ msgTo(MessageType::ADD_TRACK), receiver->track().release() });
  }

  void Controller::OnCreateMeetingSuccess(std::string meetingId, int64_t timePoint) {
    std::tuple<std::string, int64_t> result = std::make_tuple(meetingId, timePoint);
    hi::PostMsg({ msgTo(MessageType::CREATE_MEETING_OK), result });
  }

  void Controller::OnJoinMeetingSuccess(int64_t timePoint) {
    getAudioInputDevInfo(audioInputDevMap);
    getAudioOutputDevInfo(audioOutputDevMap);
    getVideoInputDevInfo(videoInputDevMap);
    getScreenInfo(shareScreenMap);
    getWindowInfo(shareWindowMap);
    //for (const auto& c : shareWindowMap) {
    //  I_LOG("{} : {}", c.first, c.second);
    //}
    hi::PostMsg({ msgTo(MessageType::AUDIO_INPUT_DEV_INFO), audioInputDevMap });
    hi::PostMsg({ msgTo(MessageType::AUDIO_OUTPUT_DEV_INFO), audioOutputDevMap });
    hi::PostMsg({ msgTo(MessageType::VIDEO_DEV_INFO), videoInputDevMap });
    hi::PostMsg({ msgTo(MessageType::SHARE_SCREEN_INFO), shareScreenMap });
    hi::PostMsg({ msgTo(MessageType::SHARE_WINDOW_INFO), shareWindowMap });
    I_LOG("[Controller::OnCreateMeetingSuccess] find dev info success");
    hi::PostMsg({ msgTo(MessageType::JOIN_MEETING_OK), timePoint });
  }

  void Controller::OnJoinMeetingFailure() {

  }

  void Controller::OnReConnectTimeout() {

  }

  //
  // VisionCnetralCallback implementation.
  //

  bool Controller::LoginSignaling(const linkinfo::ServerInfo& server, const linkinfo::UserInfo& user) {
    //调用信令接口实现登录
    if (!connect(server.serverIp_, server.serverPort_)) {
      W_LOG("[Controller::StartLogin] link server {}:{} failed", server.serverIp_, server.serverPort_);
      return false;
    }
    if (!login(user.id_, user.pwd_)) {
      W_LOG("[Controller::StartLogin] {} login failed", user.id_);
      return false;
    }
    I_LOG("[Controller::StartLogin] login user:{} to {}:{} done",
      user.id_, server.serverIp_, server.serverPort_);
    return true;
  }

  void Controller::DisconnectFromServer() {
    //TODO:待连接引擎实现断开服务器功能
  }

  bool Controller::CreateMeeting(const std::wstring& videoEnc, const std::wstring& audioEnc,
    const std::wstring& videoMcu, const std::wstring& audioMcu) {
    //用户触发
    VideoCodecType vc = VideoCodecType::H264;
    AudioCodecType ac = AudioCodecType::OPUS;
    VideoMcu vm = VideoMcu::J;
    AudioMcu am = AudioMcu::X;
    //if (videoEnc == L"vp9") {
    //  vc = VideoCodecType::VP9;
    //  I_LOG("choose vp9");
    //}
    if (audioEnc == L"pcma") {
      ac = AudioCodecType::PCMA;
      I_LOG("choose pcma");
    }
    if (videoMcu == L"Y组") {
      vm = VideoMcu::Y;
      I_LOG("choose Y组");
    }
    if (audioMcu == L"L组") {
      am = AudioMcu::L;
      I_LOG("choose L组");
    }
    createMeeting(vm, am, vc, ac);
    getAudioInputDevInfo(audioInputDevMap);
    getAudioOutputDevInfo(audioOutputDevMap);
    getVideoInputDevInfo(videoInputDevMap);
    getScreenInfo(shareScreenMap);
    getWindowInfo(shareWindowMap);
    hi::PostMsg({ msgTo(MessageType::AUDIO_INPUT_DEV_INFO), audioInputDevMap });
    hi::PostMsg({ msgTo(MessageType::AUDIO_OUTPUT_DEV_INFO), audioOutputDevMap });
    hi::PostMsg({ msgTo(MessageType::VIDEO_DEV_INFO), videoInputDevMap });
    hi::PostMsg({ msgTo(MessageType::SHARE_SCREEN_INFO), shareScreenMap });
    hi::PostMsg({ msgTo(MessageType::SHARE_WINDOW_INFO), shareWindowMap });
    I_LOG("[Controller::CreateMeeting] join meeting {} start ...", meetId_);
    return true;
  }

  bool Controller::JoinMeeting(const std::string& to) {
    //用户触发
    meetId_ = to;
    if (!joinMeeting(meetId_)) {
      E_LOG("[Controller::JoinMeeting] join meeting failed");
      return false;
    }
    I_LOG("[Controller::JoinMeeting] join meeting {} start ...", meetId_);
    return true;
  }

  void Controller::DisconnectFromCurrentPeer() {
    vision_->stopRemoteRenderer();
    if (exitMeeting()) {
      meetId_.clear();
      I_LOG("delete caller peer connection");
    }
  }

  void Controller::CustomMessageCallback(const Message& msg) {
    switch (msg.id) {
    case msgTo(MessageType::SWITCH_AUDIO_INPUT_STR): {
      std::string device = std::any_cast<std::string>(msg.data);
      for (const auto& [id, name] : audioInputDevMap) {
        if (device == name) {
          I_LOG("[Controller::CustomMessageCallback] pick mic input device:{}", name);
          setMicphone(id);
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
          setSpeaker(id);
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
          setCamera(id);
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
          setScreen(id);
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
            setWindow(id);
            break;
          }
        }
        else {
          if (label == name) {
            I_LOG("[Controller::CustomMessageCallback] pick share window {}:{}", id, name);
            setWindow(id);
            break;
          }
        }
      }
      break;
    }
    case msgTo(MessageType::SWITCH_MIC_VOLUME): {
      int volume = std::any_cast<int>(msg.data);
      I_LOG("[Controller::CustomMessageCallback] current mic volume={}", volume);
      setMicphoneVolume(volume);
      break;
    }
    case msgTo(MessageType::SET_MIC_PHONE): {
      bool state = std::any_cast<bool>(msg.data);
      I_LOG("[Controller::CustomMessageCallback] set micphone state {}", state);
      if (state) openMicphone();
      else closeMicphone();
      break;
    }
    case msgTo(MessageType::SET_CAMERA): {
      bool state = std::any_cast<bool>(msg.data);
      if (state) {
        openCamera();
      }
      else {
        closeCamera();
      }
      break;
    }
    case msgTo(MessageType::SET_SHARE): {
      bool state = std::any_cast<bool>(msg.data);
      if (state) {
        openScreenShare();
      }
      else {
        closeScreenShare();
      }
      break;
    }
    case msgTo(MessageType::REQUEST_IFRAME): {
      I_LOG("request I Frame invaild");
      //videoEngine.requestKeyFrame();
      break;
    }
    case msgTo(MessageType::REQUEST_WINDOW_LIST): {
      std::map<int, std::string> tmp = shareWindowMap;
      getWindowInfo(shareWindowMap);
      if(tmp.size() != shareWindowMap.size())
        hi::PostMsg({ msgTo(MessageType::SHARE_WINDOW_INFO), shareWindowMap });
      break;
    }
    case msgTo(MessageType::DISCONNECT_PEER): {
      //if (peerConnection_.get()) {
      //  DeletePeerConnection();
      //  I_LOG("delete callee peer connection");
      //}
      break;
    }
    case msgTo(MessageType::RECONNECT_PEER): {
      //if (peerConnection_.get()) {
      //  DeletePeerConnection(false);
      //  I_LOG("renegotiation peer connection");
      //}
      //if (!meetId_.empty()) ConnectToPeer(meetId_);
      break;
    }
    default:
      break;
    }
  }
}