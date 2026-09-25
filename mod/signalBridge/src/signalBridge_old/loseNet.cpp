//
// Created by 姚惠晶 on 2024/12/17.
//
#include <iostream>
#include "signalBridge/connectEngine.h"
#include "conductor.h"
#include "seeker/logger.h"
#include "seeker/loggerApi.h"

class MyConnect : public ConnectEngine {
public:
    std::string offerSdp = "";
    std::string answerJsep = "";
    std::atomic<bool> reConnect = false;

    void OnGenerated(std::string type_, std::string sdp_) override {
        offerSdp = sdp_;
        std::string answerSdp = "v=0\r\no=- 8512616513613472603 2 IN IP4 1.1.1.1\r\ns=-\r\nt=0 0\r\nm=audio 61000 RTP/AVP 111 63 9 102 0 8 13 110 126\r\nc=IN IP4 10.4.6.151\r\na=sendrecv\r\na=mid:0\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:2 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=msid:stream_id audio_label\r\na=rtpmap:8 PCMA/8000\r\nm=video 61002 RTP/AVP 96 98 100 39\r\nc=IN IP4 10.4.6.151\r\na=sendrecv\r\na=mid:1\r\na=extmap:14 urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:2 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\r\na=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\r\na=msid:stream_id video_label\r\na=rtpmap:96 VP8/90000\r\na=rtcp-fb:96 goog-remb\r\na=rtcp-fb:96 transport-cc\r\na=rtcp-fb:96 ccm fir\r\na=rtcp-fb:96 nack\r\na=rtcp-fb:96 nack pli\r\na=rtpmap:98 VP9/90000\r\na=rtcp-fb:98 goog-remb\r\na=rtcp-fb:98 transport-cc\r\na=rtcp-fb:98 ccm fir\r\na=rtcp-fb:98 nack\r\na=rtcp-fb:98 nack pli\r\na=fmtp:98 profile-id=0\r\na=rtpmap:100 VP9/90000\r\na=rtcp-fb:100 goog-remb\r\na=rtcp-fb:100 transport-cc\r\na=rtcp-fb:100 ccm fir\r\na=rtcp-fb:100 nack\r\na=rtcp-fb:100 nack pli\r\na=fmtp:100 profile-id=2\r\na=rtpmap:39 AV1/90000\r\na=rtcp-fb:39 goog-remb\r\na=rtcp-fb:39 transport-cc\r\na=rtcp-fb:39 ccm fir\r\na=rtcp-fb:39 nack\r\na=rtcp-fb:39 nack pli\r\na=fmtp:39 level-idx=5;profile=0;tier=0\r\n";
        sendProcess("answer", answerSdp);;
    }

    void OnProcessed(std::string type_, std::string jsep_) override {
        answerJsep = jsep_;
    }

    void OnLoseWSConnect() override {
        while (!WS_CONNECT) {
            I_LOG("try to reConnect Janus");
            ConnectEngine::connectToJanus("10.1.29.246", 8188);
            Sleep(1000);
        }
    }

    void OnWSReConnect() override {
        reConnect = true;
    }
};

int main(int argc, char *argv[]) {
    seeker::Logger::init();
    oatpp::base::Environment::init();
    std::shared_ptr<MyConnect> connect = std::make_shared<MyConnect>();
    connect->connectToJanus("10.1.29.246", 8188);
    connect->createSession();
    connect->attachNoSIP();
    auto conductor = rtc::make_ref_counted<Conductor>(connect);
    if (!conductor->InitializePeerConnection()) {
        E_LOG("InitializePeerConnection error");
    } else {
        I_LOG("InitializePeerConnection success");

        std::string jsep = "";
        while (jsep == "") {
            jsep = conductor->getJsep();
        }

        connect->sendGenerate("offer", jsep);

        while (connect->answerJsep == "") {
            Sleep(2);
        }
        conductor->setRemote(connect->answerJsep);
        I_LOG("setRemote done");
    }

    while (true) {
        if (connect->reConnect) {
            connect->answerJsep = "";
            conductor->DeletePeerConnection();

            connect->createSession();
            connect->attachNoSIP();
            if (!conductor->InitializePeerConnection()) {
                E_LOG("InitializePeerConnection error");
            } else {
                I_LOG("InitializePeerConnection success");

                std::string jsep1 = "";
                while (jsep1 == "") {
                    jsep1 = conductor->getJsep();
                }

                connect->sendGenerate("offer", jsep1);

                while (connect->answerJsep == "") {
                    Sleep(2);
                }
                conductor->setRemote(connect->answerJsep);
                I_LOG("setRemote done");

                connect->reConnect = false;
            }
        }
        else{
            Sleep(5000);
        }
    }

    oatpp::base::Environment::destroy();

    return 0;
}