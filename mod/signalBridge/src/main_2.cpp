//
// Created by 姚惠晶 on 2025/1/7.
//

#include "signalBridge/rtcConnectEngine.h"
#include "seeker/logger.h"

using namespace rtcengine;


class SignalBridge : public RtcConnectEngine {
public:
    void OnLoginSuccess(std::string userId) override {
        I_LOG("OnLoginSuccess, userId: {}", userId);
        Sleep(5000);
        createMeeting(VideoMcu::J, AudioMcu::X, VideoCodecType::H264, AudioCodecType::OPUS);
    }

    void OnLoginFailure() override {
        I_LOG("OnLoginFailure");
    }

    void OnLogoutSuccess() override {
        I_LOG("OnLogoutSuccess");
        Sleep(5000);
        login("3185", "yhj");
    }

    void OnCreateMeetingSuccess(std::string meetingId, int64_t timePoint) override {
        I_LOG("OnCreateMeetingSuccess, meetingId: {}", meetingId);
    }

    void OnReceiveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override {
        I_LOG("OnReceiveTrack");
    }

    void OnJoinMeetingSuccess(int64_t timePoint_) override {
        I_LOG("timePoint: {}", timePoint_);

        MediaInfo info;
        int i = 0;
        while( i < 10 ){
            getMediaInfo(info);
            Sleep(1000);
            i++;
        }

        outbound("3317");


        //closeMeeting();
    }

    void OnJoinMeetingFailure() override {
        I_LOG("OnJoinMeetingFailure");
    }

    void OnReConnectTimeout() override {
        I_LOG("OnReConnectTimeout");
    }

    void OnOutboundSuccess() override {
        I_LOG("OnOutboundSuccess");

        Sleep(5000);

        closeMeeting();
    }

    void OnOutboundFailure() override {
        I_LOG("OnOutboundFailure");
    }

    //预定会议
    void OnScheduleMeeting(std::string& meetingId_) {
        I_LOG("OnScheduleMeeting, meetingId: {}", meetingId_);
        Sleep(5000);
        joinMeeting(meetingId_);
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
    std::map<int16_t, std::string> micList;
    std::map<int16_t, std::string> speakerList;
    std::map<int16_t, std::string> camList;
    std::map<int, std::string> screenList;
    std::map<int, std::string> windowList;
    auto engine = rtc::make_ref_counted<SignalBridge>();

//    int ret = engine->connect("10.1.69.7", 50505);     // J内网
    //int ret = engine->connect("123.56.108.66", 30140);   // J公网
    int ret = engine->connect("10.1.29.247", 32101);   // X内网
    //int ret = engine->connect("47.93.119.6", 30150);   // X公网


    Sleep(5000);
    if (ret) {
        engine->login("3185", "yhj");



//        Sleep(20000);
//
//        engine->openCamera();
//
//        Sleep(1000);
//
//        engine->openMicphone();
//
//        Sleep(3000);
//
//        engine->closeCamera();
//
//        Sleep(1000);
//
//        engine->closeMicphone();
//
//        Sleep(5000);
//
//        engine->openScreenShare();
//
//        Sleep(2000);
//
//        engine->closeScreenShare();
//
//        Sleep(2000);
//
//        engine->getAudioInputDevInfo(micList);
//        for (const auto& pair : micList) {
//            I_LOG("mic: {} {}", pair.first, pair.second);
//        }
//
//        engine->getAudioOutputDevInfo(speakerList);
//        for (const auto& pair : speakerList) {
//            I_LOG("speaker: {} {}", pair.first, pair.second);
//        }
//
//        engine->getVideoInputDevInfo(camList);
//        for (const auto& pair : camList) {
//            I_LOG("cam: {} {}", pair.first, pair.second);
//        }
//
//        engine->getScreenInfo(screenList);
//        for (const auto& pair : screenList) {
//            I_LOG("screen: {} {}", pair.first, pair.second);
//        }
//
//        engine->getWindowInfo(windowList);
//        for (const auto& pair : windowList) {
//            I_LOG("window: {} {}", pair.first, pair.second);
//        }
//
//
//        Sleep(20000);
//
//        engine->exitMeeting();
//        I_LOG("exitMeeting");
    }
    else {
        E_LOG("connect to signaling failure");
    }

//    Sleep(2000);

    while(1){
        Sleep(1000);
    }

    return 0;
}