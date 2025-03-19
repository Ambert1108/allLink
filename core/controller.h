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
    void OnLoginSuccess(std::string userId) override;
    // 登陆失败
    void OnLoginFailure() override;
    // 登出成功
    void OnLogoutSuccess() override;
    // OnAddTrack, 即创建会议成功
    void OnReceiveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
    // 创建会议成功
    void OnCreateMeetingSuccess(std::string meetingId, int64_t timePoint) override;
    // 加入会议成功
    void OnJoinMeetingSuccess(int64_t timePoint) override;
    // 加入会议失败
    void OnJoinMeetingFailure() override;
    //断网后尝试重新连接成功
    void OnReconnectSuccess() override;
    // 断网后尝试重新连接失败
    void OnReconnectFailure() override;
    // 断网后尝试重新连接超时
    void OnReConnectTimeout() override;
    //预定会议成功
    void OnScheduleMeeting(std::string& meetingId) override;
    //预定会议失败
    void OnScheduleMeetingFailure() override;
    //结束已结束
    void OnCloseMeeting() override;

    //
    // VisionCnetralCallback implementation.
    //

    bool LoginSignaling(const linkinfo::ServerInfo& server, const linkinfo::UserInfo& user) override;

    void DisconnectFromServer() override;

    bool CreateMeeting(const std::wstring& videoEnc, const std::wstring& audioEnc, const std::wstring& videoMcu, const std::wstring& audioMcu) override;
    
    void BookingMeeting(const std::wstring& videoEnc, const std::wstring& audioEnc, const std::wstring& videoMcu, const std::wstring& audioMcu) override;

    bool JoinMeeting(const std::string& to) override;

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