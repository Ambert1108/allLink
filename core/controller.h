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
    virtual ~Controller();

    //
    // RtcConnectEngine implementation.
    //

    // 登录成功
    virtual void OnLoginSuccess(std::string userId) override;
    // 登陆失败
    virtual void OnLoginFailure() override;
    // OnAddTrack, 即入会成功
    virtual void OnReceiveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
    // 入会成功
    virtual void OnJoinMeetingSuccess(int64_t timePoint) override;
    // 入会失败
    virtual void OnJoinMeetingFailure() override;

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