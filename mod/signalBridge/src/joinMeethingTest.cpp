//
// Created by 姚惠晶 on 2025/2/24.
//
#include "signalBridge/rtcConnectEngine.h"
#include "seeker/logger.h"

using namespace rtcengine;
namespace {
    std::string mId;
}
class SignalBridge : public RtcConnectEngine {
public:
    void OnLoginSuccess(std::string userId) override {
        I_LOG("OnLoginSuccess, userId: {}", userId);
        Sleep(5000);
        createMeeting(VideoMcu::J, AudioMcu::X, VideoCodecType::H264, AudioCodecType::OPUS);
        //I_LOG(" scheduleMeeting");
        //scheduleMeeting(VideoMcu::J, AudioMcu::X, VideoCodecType::H264, AudioCodecType::OPUS);
    }

    void OnLoginFailure() override {
        I_LOG("OnLoginFailure");
    }

    void OnLogoutSuccess() override{
        I_LOG("OnLogoutSuccess");
        Sleep(5000);
        login("3283", "tjt");
    }

    void OnCreateMeetingSuccess(std::string meetingId, int64_t timePoint) override{
        I_LOG("OnCreateMeetingSuccess, meetingId: {}", meetingId);
    }

    void OnReceiveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override{
        I_LOG("OnReceiveTrack");
    }

    void OnJoinMeetingSuccess(int64_t timePoint_) override{
        I_LOG("timePoint: {}", timePoint_);
        Sleep(120000);
        //closeMeeting();
    }

    void OnJoinMeetingFailure() override{
        I_LOG("OnJoinMeetingFailure");
    }

    void OnReConnectTimeout() override{
        I_LOG("OnReConnectTimeout");
    }
    //预定会议
    void OnScheduleMeeting(std::string& meetingId_) {
        I_LOG("OnScheduleMeeting, meetingId: {}", meetingId_);
        Sleep(5000);
        //joinMeeting(meetingId_);
    }
    //预定会议失败
    void OnScheduleMeetingFailure() {
        I_LOG("OnScheduleMeetingFailure");
    }
    //告知 与信令服务器重连成功
    void OnReconnectSuccess() {
        I_LOG("OnReconnectSuccess");

    }
    //告知 与信令服务器重连失败
    void OnReconnectFailure() {
        I_LOG("OnReconnectFailure");
    }
    void OnCloseMeeting() {
        I_LOG("OnCloseMeeting");
    }
};

int main(int argc, char *argv[]){
    seeker::Logger::init();
    auto engine = rtc::make_ref_counted<SignalBridge>();

    int ret = engine->connect("10.1.69.7", 50505);     // J内网
    //int ret = engine->connect("123.56.108.66", 30150);
    //int ret = engine->connect("123.56.108.66", 30140);
    I_LOG("CONNECT success");
    Sleep(5000);
    if (ret) {
        engine->login("3283", "tjt");
 /*       while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }*/
        Sleep(600000);

        //engine->exitMeeting();
        //I_LOG("exitMeeting");
        //
       // I_LOG("engine->scheduleMeeting");
        //engine->scheduleMeeting(RtcConnectEngine::VideoMcu::J, RtcConnectEngine::AudioMcu::X, RtcConnectEngine::VideoCodecType::H264, RtcConnectEngine::AudioCodecType::OPUS);
        //Sleep(5000);
        
        //engine->createMeeting(RtcConnectEngine::VideoMcu::J, RtcConnectEngine::AudioMcu::X, RtcConnectEngine::VideoCodecType::H264, RtcConnectEngine::AudioCodecType::OPUS);
        
        //engine->exitMeeting();
        //I_LOG("exitMeeting");
    }
    else {
        E_LOG("connect to signaling failure");
    }


    return 0;
}