//
// Created by 姚惠晶 on 2025/2/24.
//
#include "signalBridge/rtcConnectEngine.h"
#include "seeker/logger.h"

using namespace rtcengine;

class SignalBridge : public RtcConnectEngine {
public:
    void OnLoginSuccess(std::string userId) override {
        I_LOG("OnLoginSuccess, userId: {}", userId);

        Sleep(5000);
        logout();
    }

    void OnLoginFailure() override {
        I_LOG("OnLoginFailure");
    }

    void OnLogoutSuccess() override{
        I_LOG("OnLogoutSuccess");
        Sleep(5000);
        login("3185", "yhj");
    }

    void OnReceiveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override{
        I_LOG("OnReceiveTrack");
    }

    void OnJoinMeetingSuccess(int64_t timePoint_) override{
        I_LOG("timePoint: {}", timePoint_);
    }

    void OnJoinMeetingFailure() override{
        I_LOG("OnJoinMeetingFailure");
    }

    void OnReConnectTimeout() override{
        I_LOG("OnReConnectTimeout");
    }
};

int main(int argc, char *argv[]){
    seeker::Logger::init();
    auto engine = rtc::make_ref_counted<SignalBridge>();

    int ret = engine->connect("10.1.69.7", 50505);     // J内网
//    int ret = engine->connect("123.56.108.66", 30140);

    Sleep(5000);
    if (ret) {
        engine->login("3185", "yhj");

        Sleep(50000);

//        engine->exitMeeting();
//        I_LOG("exitMeeting");


    }
    else {
        E_LOG("connect to signaling failure");
    }


    return 0;
}