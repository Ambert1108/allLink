//
// Created by 姚惠晶 on 2024/11/20.
//
#pragma once

#include <iostream>

#include "seeker/loggerApi.h"
#include "seeker/json.hpp"
#include "nlohmann/fifo_map.hpp"
#include "WSListener.h"
#include "conductor.h"

#include "oatpp-websocket/WebSocket.hpp"
#include "oatpp-websocket/Connector.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/win32_socket_init.h"
#include "rtc_base/thread.h"
#include "rtc_base/ssl_adapter.h"
#include "api/make_ref_counted.h"

template<class K, class V, class dummy_compare, class A>
using my_workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<my_workaround_fifo_map>;

namespace {
    const char *TAG = "signalBridge_old";

    std::string JanusIp = "";
    v_uint16 JanusPort = 0;

    std::string signallingIp = "";
    v_uint16 signallingPort = 0;

    int cseq = 0;

    bool finished = false;

//    static int cseq = 0;

    std::shared_ptr<engineListener> listener = nullptr;

    void socketTask(const std::shared_ptr<oatpp::websocket::WebSocket> &websocket) {
        websocket->listen();
        finished = true;
    }

}

void JanusRun() {
    OATPP_LOGI(TAG, "start connect to Janus");
    auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({JanusIp, JanusPort});
    I_LOG("{}:{}", JanusIp, JanusPort);
    auto connector = oatpp::websocket::Connector::createShared(connectionProvider);
    oatpp::websocket::Connector::Headers header;
    header.put("Sec-WebSocket-Protocol", "janus-protocol");
    auto connection = connector->connect("/", header);
    OATPP_LOGI(TAG, "Janus connected done!");

    auto JanusSocket = oatpp::websocket::WebSocket::createShared(connection,
                                                                 true /* maskOutgoingMessages must be true for clients */);
    std::mutex socketWriteMutex;
    listener = std::make_shared<engineListener>(socketWriteMutex);
    JanusSocket->setListener(listener);

//    int cseq = 0;
    std::thread thread(socketTask, JanusSocket);
    std::lock_guard<std::mutex> lock(socketWriteMutex);
    json create;
    create["janus"] = "create";
    create["transaction"] = std::to_string(cseq);
    I_LOG("create Req: {}", create.dump(4));
    oatpp::String createJson = oatpp::String(create.dump());
    JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, createJson);

    json createResp;
    while (1) {
        createResp = listener->getResponse();
        if (createResp != nullptr && std::atoi(createResp["transaction"].get<std::string>().c_str()) == cseq) {
            break;
        } else {
            Sleep(2);
        }
    }
    long long session_id = createResp["data"]["id"].get<long long>();
    I_LOG("create Resp: {}", createResp.dump(4));
    cseq++;

    json attach;
    attach["janus"] = "attach";
    attach["plugin"] = "janus.plugin.nosip";
    attach["opaque_id"] = "user1";
    attach["session_id"] = session_id;
    attach["transaction"] = std::to_string(cseq);
    I_LOG("attach Req: {}", attach.dump(4));
    oatpp::String attachJson = oatpp::String(attach.dump());
    JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, attachJson);
    json attachResp;
    while (1) {
        attachResp = listener->getResponse();
        if (attachResp != nullptr && std::atoi(attachResp["transaction"].get<std::string>().c_str()) == cseq) {
            break;
        } else {
            Sleep(2);
        }
    }
    long long handle_id = attachResp["data"]["id"].get<long long>();
    I_LOG("attach Resp: {}", attachResp.dump(4));
    cseq++;

    auto conductor = rtc::make_ref_counted<Conductor>(JanusSocket, session_id, handle_id);
    if (!conductor->InitializePeerConnection()) {
        E_LOG("InitializePeerConnection error");
    } else {
        I_LOG("InitializePeerConnection success");

        std::string jsep;
        while (1) {
            jsep = conductor->getJsep();
            if (jsep != "") {
                break;
            } else {
                Sleep(2);
            }
        }

        json generate;
        generate["janus"] = "message";
        generate["body"]["request"] = "generate";
        generate["transaction"] = std::to_string(cseq);
        generate["jsep"]["type"] = "offer";
        generate["jsep"]["sdp"] = jsep;
        generate["session_id"] = session_id;
        generate["handle_id"] = handle_id;
        I_LOG("generate Req: {}", generate.dump(4));
        oatpp::String generateJson = oatpp::String(generate.dump());
        JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, generateJson);
        json generated;
        while (1) {
            generated = listener->getResponse();
            if (generated != nullptr &&
                std::atoi(generated["transaction"].get<std::string>().c_str()) == cseq &&
                generated["janus"] == "event") {
                break;
            } else {
                Sleep(2);
            }
        }
        std::string sdp = generated["plugindata"]["data"]["result"]["sdp"].get<std::string>();
        I_LOG("generate Resp: {}", generated.dump(4));
        cseq++;

        Sleep(20);

        // audio
//        std::string answerSdp = "v=0\r\no=- 4335836207531891234 2 IN IP4 1.1.1.1\r\ns=-\r\nt=0 0\r\nm=audio 61000 RTP/AVP 111 63 9 102 0 8 13 110 126\r\nc=IN IP4 10.4.6.151\r\na=sendrecv\r\na=mid:0\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:2 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=msid:stream_id audio_label\r\na=rtpmap:8 PCMA/8000\r\n";
        // audio + video
//        std::string answerSdp = "v=0\r\no=- 8512616513613472603 2 IN IP4 1.1.1.1\r\ns=-\r\nt=0 0\r\nm=audio 61000 RTP/AVP 111 63 9 102 0 8 13 110 126\r\nc=IN IP4 10.4.6.151\r\na=sendrecv\r\na=mid:0\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:2 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=msid:stream_id audio_label\r\na=rtpmap:8 PCMA/8000\r\nm=video 61002 RTP/AVP 96 98 100 39\r\nc=IN IP4 10.4.6.151\r\na=sendrecv\r\na=mid:1\r\na=extmap:14 urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:2 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\r\na=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\r\na=msid:stream_id video_label\r\na=rtpmap:96 VP8/90000\r\na=rtcp-fb:96 goog-remb\r\na=rtcp-fb:96 transport-cc\r\na=rtcp-fb:96 ccm fir\r\na=rtcp-fb:96 nack\r\na=rtcp-fb:96 nack pli\r\na=rtpmap:98 VP9/90000\r\na=rtcp-fb:98 goog-remb\r\na=rtcp-fb:98 transport-cc\r\na=rtcp-fb:98 ccm fir\r\na=rtcp-fb:98 nack\r\na=rtcp-fb:98 nack pli\r\na=fmtp:98 profile-id=0\r\na=rtpmap:100 VP9/90000\r\na=rtcp-fb:100 goog-remb\r\na=rtcp-fb:100 transport-cc\r\na=rtcp-fb:100 ccm fir\r\na=rtcp-fb:100 nack\r\na=rtcp-fb:100 nack pli\r\na=fmtp:100 profile-id=2\r\na=rtpmap:39 AV1/90000\r\na=rtcp-fb:39 goog-remb\r\na=rtcp-fb:39 transport-cc\r\na=rtcp-fb:39 ccm fir\r\na=rtcp-fb:39 nack\r\na=rtcp-fb:39 nack pli\r\na=fmtp:39 level-idx=5;profile=0;tier=0\r\n";
        std::string answerSdp = "v=0\r\no=- 4262987656738261604 2 IN IP4 1.1.1.1\r\ns=-\r\nt=0 0\r\nm=audio 61000 RTP/AVP 111 63 9 102 0 8 13 110 126\r\nc=IN IP4 10.4.6.151\r\na=sendrecv\r\na=mid:0\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=msid:stream_id audio_label\r\na=rtpmap:8 PCMA/8000\r\nm=video 61002 RTP/AVP 96 98 100 39\r\nc=IN IP4 10.4.6.151\r\na=sendrecv\r\na=mid:1\r\na=extmap:14 urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:13 urn:3gpp:video-orientation\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay\r\na=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type\r\na=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing\r\na=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\r\na=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\r\na=msid:stream_id video_label\r\na=rtpmap:96 VP8/90000\r\na=rtcp-fb:96 goog-remb\r\na=rtcp-fb:96 transport-cc\r\na=rtcp-fb:96 ccm fir\r\na=rtcp-fb:96 nack\r\na=rtcp-fb:96 nack pli\r\na=rtpmap:98 VP9/90000\r\na=rtcp-fb:98 goog-remb\r\na=rtcp-fb:98 transport-cc\r\na=rtcp-fb:98 ccm fir\r\na=rtcp-fb:98 nack\r\na=rtcp-fb:98 nack pli\r\na=fmtp:98 profile-id=0\r\na=rtpmap:100 VP9/90000\r\na=rtcp-fb:100 goog-remb\r\na=rtcp-fb:100 transport-cc\r\na=rtcp-fb:100 ccm fir\r\na=rtcp-fb:100 nack\r\na=rtcp-fb:100 nack pli\r\na=fmtp:100 profile-id=2\r\na=rtpmap:39 AV1/90000\r\na=rtcp-fb:39 goog-remb\r\na=rtcp-fb:39 transport-cc\r\na=rtcp-fb:39 ccm fir\r\na=rtcp-fb:39 nack\r\na=rtcp-fb:39 nack pli\r\na=fmtp:39 level-idx=5;profile=0;tier=0\r\n";
        json process;
        process["janus"] = "message";
        process["body"]["request"] = "process";
        process["body"]["type"] = "answer";
        process["body"]["sdp"] = answerSdp.c_str();
        process["transaction"] = std::to_string(cseq);
        process["session_id"] = session_id;
        process["handle_id"] = handle_id;
        I_LOG("process Req: {}", process.dump(4));
        oatpp::String processJson = oatpp::String(process.dump());
        JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, processJson);
        json processed;
        while (1) {
            processed = listener->getResponse();
            if (processed != nullptr &&
                std::atoi(processed["transaction"].get<std::string>().c_str()) == cseq &&
                processed["janus"] == "event") {
                break;
            } else {
                Sleep(2);
            }
        }
        std::string answerJsep = processed["jsep"]["sdp"].get<std::string>();
        I_LOG("process Resp: {}", processed.dump(4));
        cseq++;

        conductor->setRemote(answerJsep);

        while(1){
            json keepalive;
            keepalive["janus"] = "keepalive";
            keepalive["session_id"] = session_id;
            keepalive["transaction"] = std::to_string(cseq);
            I_LOG("keepalive Req: {}", keepalive.dump(4));
            oatpp::String keepaliveJson = oatpp::String(keepalive.dump());
            JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, keepaliveJson);
            Sleep(25000);
            cseq++;
        }
    }
    thread.join();
}

void signallingRun() {
    OATPP_LOGI(TAG, "start connect to signalling");
    auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared(
            {signallingIp, signallingPort});
    auto connector = oatpp::websocket::Connector::createShared(connectionProvider);
    auto connection = connector->connect("/connectWS");
    OATPP_LOGI(TAG, "signalling connected done!");

    auto signalSocket = oatpp::websocket::WebSocket::createShared(connection,
                                                                  true /* maskOutgoingMessages must be true for clients */);
    std::mutex socketWriteMutex;
    signalSocket->setListener(std::make_shared<engineListener>(socketWriteMutex));
    std::thread thread(socketTask, signalSocket);

    int cseq = 0;
    std::lock_guard<std::mutex> lock(socketWriteMutex);
    json j;
    j["meth"] = "REGISTER";
    j["isResponse"] = false;
    j["userId"] = "3185";
    j["password"] = "yhj";
    j["Cseq"] = cseq++;

    oatpp::String registerJson = oatpp::String(j.dump());

    signalSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, registerJson);

    signalSocket->sendClose();
    thread.join();
}

int main() {
    oatpp::base::Environment::init();
    JanusIp = "10.1.29.246";
    JanusPort = 8188;

    signallingIp = "10.1.29.247";
    signallingPort = 32103;

//    signallingRun();
    JanusRun();

    oatpp::base::Environment::destroy();

    return 0;
}