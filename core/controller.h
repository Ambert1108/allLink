#pragma once

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "seeker/logger.h"
#include "seeker/loggerApi.h"
#include "seeker/iniConfig.hpp"
#include "engine/signalBridge/rtcConnectEngine.h"

#include "vision.h"


namespace alllink {
  class Controller :
    public VisionCentralCallback,
    public rtcengine::RtcConnectEngine {

  public:
    Controller(VisionCentralBase* vcb);

    void Close() override;

  protected:
    ~Controller();

    //
    // RtcConnectEngine implementation.
    //

    // 登录成功
    virtual void OnLoginSuccess(std::string userId) override;
    // 登陆失败
    virtual void OnLoginFailure() override;
    // OnAddTrack, 即入会成功
    virtual void OnReceiveTrack(webrtc::MediaStreamTrackInterface* receiver) override;
    // 入会成功
    virtual void OnJoinMeetingSuccess(int64_t timePoint) override;
    // 入会失败
    virtual void OnJoinMeetingFailure() override;
    // 获取到麦克风设备信息
    virtual void OnAudioInputDevInfo(std::map<int16_t, std::string> list) override;
    // 获取扬声器设备信息
    virtual void OnAudioOutputDevInfo(std::map<int16_t, std::string> list) override;
    // 获取摄像头设备信息
    virtual void OnVideoInputDevInfo(std::map<int16_t, std::string> list) override;
    // 获取屏幕设备信息
    virtual void OnScreenInfo(std::map<int, std::string> list) override;
    // 获取窗口信息
    virtual void OnWindowInfo(std::map<int, std::string> list) override;

    //
    // VisionCnetralCallback implementation.
    //

    bool LoginSignaling(const linkinfo::ServerInfo& server, const linkinfo::UserInfo& user) override;

    void DisconnectFromServer() override;

    bool ConnectToPeer(const std::string& to) override;

    void DisconnectFromCurrentPeer() override;

    void CustomMessageCallback(const Message& msg) override;

  private:
    VisionCentralBase* vision_;
    std::map<int16_t, std::string> audioInputDevMap, audioOutputDevMap;
    std::map<int16_t, std::string> videoInputDevMap;
    std::map<int, std::string> shareScreenMap, shareWindowMap;
    std::string meetId_;
    bool needRequestIFrame = false;
  };
}