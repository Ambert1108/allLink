#include <iostream>

#include "seeker/logger.h"
#include "seeker/loggerApi.h"
#include "seeker/json.hpp"
#include "nlohmann/fifomap.hpp"
#include "wslistener.h"

#include "oatpp-websocket/WebSocket.hpp"
#include "oatpp-websocket/Connector.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"

#include <regex>

template<class K, class V, class dummy_compare, class A>
using my_workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<my_workaround_fifo_map>;

std::shared_ptr<oatpp::websocket::WebSocket> JanusSocket;
std::shared_ptr<alllink::WSListener> listener;

namespace {
    const char* TAG = "nosip";

    std::string JanusIp = "";
    v_uint16 JanusPort = 0;

    bool finished = false;
    int64_t cseq = 0;
    int64_t sessionId = 0;
    int64_t handleId = 0;

    void socketTask(const std::shared_ptr<oatpp::websocket::WebSocket>& websocket) {
        websocket->listen();
        finished = true;
    }
}

void JanusRun(){
    OATPP_LOGI(TAG, "start connect to Janus");
    auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({JanusIp, JanusPort});
    I_LOG("{} : {}", JanusIp, JanusPort);
    OATPP_LOGI(TAG, "start connect to Janus 1");
    auto connector = oatpp::websocket::Connector::createShared(connectionProvider);
    OATPP_LOGI(TAG, "start connect to Janus 2");
    oatpp::websocket::Connector::Headers header;
    header.put("Sec-WebSocket-Protocol", "janus-protocol");
    auto connection = connector->connect("/", header);
    OATPP_LOGI(TAG, "Janus connected done!");

    JanusSocket = oatpp::websocket::WebSocket::createShared(connection, true);
    listener = std::make_shared<alllink::WSListener>();
    JanusSocket->setListener(listener);
    std::thread listen(socketTask, JanusSocket);

    json j1;
    j1["janus"] = "create";
    j1["transaction"] = std::to_string(cseq++);
    I_LOG("send msg:{}", j1.dump(4));
    oatpp::String registerJson = oatpp::String(j1.dump());

    JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, registerJson);

    oatpp::String msg;
    while (listener->getMsg(msg) != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    json js = json::parse(msg->c_str());
    sessionId = js["data"]["id"];

    json j2;
    j2["janus"] = "attach";
    j2["session_id"] = sessionId;
    j2["plugin"] = "janus.plugin.nosip";
    j2["transaction"] = std::to_string(cseq++);
    registerJson = oatpp::String(j2.dump());
    I_LOG("send msg:{}", j2.dump(4));
    JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, registerJson);
    while (listener->getMsg(msg) != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    js = json::parse(msg->c_str());
    handleId = js["data"]["id"];

    json j3;
    j3["janus"] = "message";
    j3["session_id"] = sessionId;
    j3["handle_id"] = handleId;
    j3["transaction"] = std::to_string(cseq++);
    j3["body"]["request"] = "process";
    j3["body"]["sdp"] = "v=0\r\no=- 847452011990778653 2 IN IP4 1.1.1.1\r\ns=-\r\nt=0 0\r\nm=audio 20418 RTP/AVP 111 63 9 102 0 8 13 110 126\r\nc=IN IP4 10.1.29.246\r\na=sendrecv\r\na=mid:0\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=msid:stream_id audio_label\r\na=rtpmap:111 opus/48000/2\r\na=rtcp-fb:111 transport-cc\r\na=fmtp:111 minptime=10;useinbandfec=1\r\na=rtpmap:63 red/48000/2\r\na=fmtp:63 111/111\r\na=rtpmap:9 G722/8000\r\na=rtpmap:102 ILBC/8000\r\na=rtpmap:0 PCMU/8000\r\na=rtpmap:8 PCMA/8000\r\na=rtpmap:13 CN/8000\r\na=rtpmap:110 telephone-event/48000\r\na=rtpmap:126 telephone-event/8000\r\nm=video 20420 RTP/AVP 96 98 100 39\r\nc=IN IP4 10.1.29.246\r\na=sendrecv\r\na=mid:1\r\na=extmap:14 urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:13 urn:3gpp:video-orientation\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay\r\na=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type\r\na=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing\r\na=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\r\na=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\r\na=msid:stream_id video_label\r\na=rtpmap:96 VP8/90000\r\na=rtcp-fb:96 goog-remb\r\na=rtcp-fb:96 transport-cc\r\na=rtcp-fb:96 ccm fir\r\na=rtcp-fb:96 nack\r\na=rtcp-fb:96 nack pli\r\na=rtpmap:98 VP9/90000\r\na=rtcp-fb:98 goog-remb\r\na=rtcp-fb:98 transport-cc\r\na=rtcp-fb:98 ccm fir\r\na=rtcp-fb:98 nack\r\na=rtcp-fb:98 nack pli\r\na=fmtp:98 profile-id=0\r\na=rtpmap:100 VP9/90000\r\na=rtcp-fb:100 goog-remb\r\na=rtcp-fb:100 transport-cc\r\na=rtcp-fb:100 ccm fir\r\na=rtcp-fb:100 nack\r\na=rtcp-fb:100 nack pli\r\na=fmtp:100 profile-id=2\r\na=rtpmap:39 AV1/90000\r\na=rtcp-fb:39 goog-remb\r\na=rtcp-fb:39 transport-cc\r\na=rtcp-fb:39 ccm fir\r\na=rtcp-fb:39 nack\r\na=rtcp-fb:39 nack pli\r\na=fmtp:39 level-idx=5;profile=0;tier=0\r\n";
    j3["body"]["type"] = "offer";
    registerJson = oatpp::String(j3.dump());
    I_LOG("send msg:{}", j3.dump(4));
    JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, registerJson);

    while (1) {
      json js;
      js["janus"] = "keepalive";
      js["session_id"] = sessionId;
      js["transaction"] = std::to_string(cseq++);
      registerJson = oatpp::String(js.dump());
      I_LOG("send msg:{}", js.dump(4));
      JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, registerJson);
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    listen.join();
}

int main(){
    oatpp::base::Environment::init();
    JanusIp = "10.1.29.246";
    JanusPort = 8188;
    
    JanusRun();
    
    oatpp::base::Environment::destroy();

  //std::string sdp = R"(v=0\r\no=- 8545059463340826434 2 IN IP4 1.1.1.1\r\ns=-\r\nt=0 0\r\nm=audio 20524 RTP/AVP 111 63 9 102 0 8 13 110 126\r\nc=IN IP4 10.1.29.246\r\na=sendrecv\r\na=mid:0\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=msid:stream_id audio_label\r\na=rtpmap:111 opus/48000/2\r\na=rtcp-fb:111 transport-cc\r\na=fmtp:111 minptime=10;useinbandfec=1\r\na=rtpmap:63 red/48000/2\r\na=fmtp:63 111/111\r\na=rtpmap:9 G722/8000\r\na=rtpmap:102 ILBC/8000\r\na=rtpmap:0 PCMU/8000\r\na=rtpmap:8 PCMA/8000\r\na=rtpmap:13 CN/8000\r\na=rtpmap:110 telephone-event/48000\r\na=rtpmap:126 telephone-event/8000\r\nm=video 20526 RTP/AVP 96 98 100 39\r\nc=IN IP4 10.1.29.246\r\na=sendrecv\r\na=mid:1\r\na=extmap:14 urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:13 urn:3gpp:video-orientation\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay\r\na=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type\r\na=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing\r\na=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\r\na=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\r\na=msid:stream_id video_label\r\na=rtpmap:96 VP8/90000\r\na=rtcp-fb:96 goog-remb\r\na=rtcp-fb:96 transport-cc\r\na=rtcp-fb:96 ccm fir\r\na=rtcp-fb:96 nack\r\na=rtcp-fb:96 nack pli\r\na=rtpmap:98 VP9/90000\r\na=rtcp-fb:98 goog-remb\r\na=rtcp-fb:98 transport-cc\r\na=rtcp-fb:98 ccm fir\r\na=rtcp-fb:98 nack\r\na=rtcp-fb:98 nack pli\r\na=fmtp:98 profile-id=0\r\na=rtpmap:100 VP9/90000\r\na=rtcp-fb:100 goog-remb\r\na=rtcp-fb:100 transport-cc\r\na=rtcp-fb:100 ccm fir\r\na=rtcp-fb:100 nack\r\na=rtcp-fb:100 nack pli\r\na=fmtp:100 profile-id=2\r\na=rtpmap:39 AV1/90000\r\na=rtcp-fb:39 goog-remb\r\na=rtcp-fb:39 transport-cc\r\na=rtcp-fb:39 ccm fir\r\na=rtcp-fb:39 nack\r\na=rtcp-fb:39 nack pli\r\na=fmtp:39 level-idx=5;profile=0;tier=0\r\n)";
  //std::regex pattern(R"(m=audio.*?(?=m=video))");
  //
  //std::smatch matches;
  //if (std::regex_search(sdp, matches, pattern)) {
  //  std::cout << matches[0] << std::endl; // 返回匹配的内容
  //}

  return 0;
}