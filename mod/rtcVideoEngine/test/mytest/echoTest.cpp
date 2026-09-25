#include<iostream>
#include <string>
#include<seeker/common.h>
#include<seeker/loggerApi.h>
#include<seeker/logger.h>
#include "seeker/http.hpp"
#include "seeker/json.hpp"
#include "utils/httplib.h"
#include "nlohmann/json.hpp"
#include "nlohmann/fifomap.hpp"

template<class K, class V, class dummy_compare, class A>
using json_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<json_fifo_map>;

int main() {
	seeker::Logger::init();
	I_LOG("hello rtcVideoEngine");
	//http://10.1.29.246:8088/janus
	/*
	[20241119 16:09:31.112 echoTest.cpp:48] [I]: [XrController::joinMeeting] respone body={
   "janus": "success",
   "transaction": "<random alphanumeric string>",
   "data": {
      "id": 5004037135516188
   }
	}
	*/

	//The server root
	std::string url = "/janus";
	httplib::Headers aheader = httplib::Headers();
	std::unique_ptr<httplib::Client> client = std::make_unique<httplib::Client>("10.1.29.246", 8088);
	client->set_connection_timeout(5, 0);
	client->set_read_timeout(7, 0);
	client->set_write_timeout(7, 0);

	json j;
	j["janus"] = "create";
	//j["plugin"] = "<the plugin's unique package name>";
	j["transaction"] = "<random string>";
	std::string reqBody = seeker::json::toJsonString(j);
	int64_t sTime = seeker::time::currentTime();
	httplib::Result res;
	I_LOG("server root reqBody ={}", reqBody);
	res = client->Post(url.c_str(), aheader, reqBody, "application/json");
	//res = client->Get(url.c_str(), aheader);
	int64_t cTime = seeker::time::currentTime() - sTime;

	if (cTime >= 1000) {
		W_LOG("[XrController::joinMeeting] Send request To vmcu Server use {}ms", cTime);
	}
	if (!res) {
		E_LOG("[XrController::joinMeeting] Send request To vmcu Server failed, errCode={}", (int)res.error());
		return -1;
	}
	if (res->status != 200) {
		E_LOG("[XrController::joinMeeting] vmcu Server response failed, status={}", res->status);
		return -1;
	}

	I_LOG("server root respone body= {}", res->body);
	json rsp;
	seeker::json::fromJsonString(rsp, res->body);
	std::string sessionId = rsp["data"]["id"].dump();
	I_LOG("sessionId:{}", sessionId);
	std::cout << std::endl;
	//The session endpoint
	url = "/janus/" + sessionId;
	/*httplib::Headers aheader = httplib::Headers();
	std::unique_ptr<httplib::Client> client = std::make_unique<httplib::Client>("10.1.29.246", 8088);
	client->set_connection_timeout(5, 0);
	client->set_read_timeout(7, 0);
	client->set_write_timeout(7, 0);

	json j;*/
	j["janus"] = "attach";
	j["transaction"] = "<random string>";
	j["plugin"] = "janus.plugin.echotest";
	reqBody = seeker::json::toJsonString(j);
	sTime = seeker::time::currentTime();
	I_LOG("session endpoint reqBody = {}", reqBody);
	res = client->Post(url.c_str(), aheader, reqBody, "application/json");
	//res = client->Get(url.c_str(), aheader);
	cTime = seeker::time::currentTime() - sTime;

	if (cTime >= 1000) {
		W_LOG("[XrController::joinMeeting] Send request To vmcu Server use {}ms", cTime);
	}
	if (!res) {
		E_LOG("[XrController::joinMeeting] Send request To vmcu Server failed, errCode={}", (int)res.error());
		return -1;
	}
	if (res->status != 200) {
		E_LOG("[XrController::joinMeeting] vmcu Server response failed, status={}", res->status);
		return -1;
	}

	I_LOG("session endpoint respone body = {}", res->body);
	
	seeker::json::fromJsonString(rsp, res->body);
	std::string pluginId = rsp["data"]["id"].dump();
	I_LOG("pluginId:{}", pluginId);
	std::cout << std::endl;

	//The plugin endpoint
	/*
	{
           "janus": "message",
           "session_id": <session_id>,
           "handle_id": <handle_id>,
           "body": {
               "request": "create",
               "room": 1234,
               "description": "Echo Test Room"
           },
           "transaction": "your_transaction_id"
         }
	*/


	url = "/janus/" + sessionId + "/" + pluginId;
	j["janus"] = "message";
	j["transaction"] = "<random string>";
	json body;
	/*body["request"] = "configure";
	body["audio"] = "true";
	body["video"] = "true";
	json jsep;
	jsep["type"] = "offer";
	jsep["sdp"] = "v=0\r\no=- 7188370618827213634 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=group:BUNDLE 0 1\r\na=msid-semantic: WMS lRp1CXMUAO275BhiToZVq6jbSE1dXSLV9hLh\r\nm=audio 9 UDP/TLS/RTP/SAVPF 111 103 104 9 0 8 106 105 13 110 112 113 126\r\nc=IN IP4 0.0.0.0\r\na=rtcp:9 IN IP4 0.0.0.0\r\na=ice-ufrag:VUb2\r\na=ice-pwd:SlG64/+DM1GUEl6btHpcKkd7\r\na=ice-options:trickle\r\na=fingerprint:sha-256 9C:A6:A3:40:AA:CE:9C:6D:73:AD:53:59:B6:7A:C5:5A:02:0D:70:49:DA:AA:6D:AC:0A:26:39:E3:E3:CF:AE:48\r\na=setup:actpass\r\na=mid:0\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=extmap:5 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\r\na=extmap:6 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\r\na=sendonly\r\na=msid:lRp1CXMUAO275BhiToZVq6jbSE1dXSLV9hLh c740d785-f057-434e-a8cf-8f231f2f6696\r\na=rtcp-mux\r\na=rtpmap:111 opus/48000/2\r\na=rtcp-fb:111 transport-cc\r\na=fmtp:111 minptime=10;useinbandfec=1\r\na=rtpmap:103 ISAC/16000\r\na=rtpmap:104 ISAC/32000\r\na=rtpmap:9 G722/8000\r\na=rtpmap:0 PCMU/8000\r\na=rtpmap:8 PCMA/8000\r\na=rtpmap:106 CN/32000\r\na=rtpmap:105 CN/16000\r\na=rtpmap:13 CN/8000\r\na=rtpmap:110 telephone-event/48000\r\na=rtpmap:112 telephone-event/32000\r\na=rtpmap:113 telephone-event/16000\r\na=rtpmap:126 telephone-event/8000\r\na=ssrc:130909908 cname:Wn97EKOJFPlcc9wm\r\na=ssrc:130909908 msid:lRp1CXMUAO275BhiToZVq6jbSE1dXSLV9hLh c740d785-f057-434e-a8cf-8f231f2f6696\r\na=ssrc:130909908 mslabel:lRp1CXMUAO275BhiToZVq6jbSE1dXSLV9hLh\r\na=ssrc:130909908 label:c740d785-f057-434e-a8cf-8f231f2f6696\r\nm=video 9 UDP/TLS/RTP/SAVPF 96 97 98 99 100 101 102 122 127 121 125 107 108 109 124 120 123\r\nc=IN IP4 0.0.0.0\r\na=rtcp:9 IN IP4 0.0.0.0\r\na=ice-ufrag:VUb2\r\na=ice-pwd:SlG64/+DM1GUEl6btHpcKkd7\r\na=ice-options:trickle\r\na=fingerprint:sha-256 9C:A6:A3:40:AA:CE:9C:6D:73:AD:53:59:B6:7A:C5:5A:02:0D:70:49:DA:AA:6D:AC:0A:26:39:E3:E3:CF:AE:48\r\na=setup:actpass\r\na=mid:1\r\na=extmap:14 urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:13 urn:3gpp:video-orientation\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:12 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay\r\na=extmap:11 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type\r\na=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing\r\na=extmap:8 http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07\r\na=extmap:9 http://www.webrtc.org/experiments/rtp-hdrext/color-space\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=extmap:5 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\r\na=extmap:6 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\r\na=sendonly\r\na=msid:lRp1CXMUAO275BhiToZVq6jbSE1dXSLV9hLh d6a76d64-297a-4b35-b813-30fe03fc6370\r\na=rtcp-mux\r\na=rtcp-rsize\r\na=rtpmap:96 VP8/90000\r\na=rtcp-fb:96 goog-remb\r\na=rtcp-fb:96 transport-cc\r\na=rtcp-fb:96 ccm fir\r\na=rtcp-fb:96 nack\r\na=rtcp-fb:96 nack pli\r\na=rtpmap:97 rtx/90000\r\na=fmtp:97 apt=96\r\na=rtpmap:98 VP9/90000\r\na=rtcp-fb:98 goog-remb\r\na=rtcp-fb:98 transport-cc\r\na=rtcp-fb:98 ccm fir\r\na=rtcp-fb:98 nack\r\na=rtcp-fb:98 nack pli\r\na=fmtp:98 profile-id=0\r\na=rtpmap:99 rtx/90000\r\na=fmtp:99 apt=98\r\na=rtpmap:100 VP9/90000\r\na=rtcp-fb:100 goog-remb\r\na=rtcp-fb:100 transport-cc\r\na=rtcp-fb:100 ccm fir\r\na=rtcp-fb:100 nack\r\na=rtcp-fb:100 nack pli\r\na=fmtp:100 profile-id=2\r\na=rtpmap:101 rtx/90000\r\na=fmtp:101 apt=100\r\na=rtpmap:102 H264/90000\r\na=rtcp-fb:102 goog-remb\r\na=rtcp-fb:102 transport-cc\r\na=rtcp-fb:102 ccm fir\r\na=rtcp-fb:102 nack\r\na=rtcp-fb:102 nack pli\r\na=fmtp:102 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f\r\na=rtpmap:122 rtx/90000\r\na=fmtp:122 apt=102\r\na=rtpmap:127 H264/90000\r\na=rtcp-fb:127 goog-remb\r\na=rtcp-fb:127 transport-cc\r\na=rtcp-fb:127 ccm fir\r\na=rtcp-fb:127 nack\r\na=rtcp-fb:127 nack pli\r\na=fmtp:127 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=42001f\r\na=rtpmap:121 rtx/90000\r\na=fmtp:121 apt=127\r\na=rtpmap:125 H264/90000\r\na=rtcp-fb:125 goog-remb\r\na=rtcp-fb:125 transport-cc\r\na=rtcp-fb:125 ccm fir\r\na=rtcp-fb:125 nack\r\na=rtcp-fb:125 nack pli\r\na=fmtp:125 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42e01f\r\na=rtpmap:107 rtx/90000\r\na=fmtp:107 apt=125\r\na=rtpmap:108 H264/90000\r\na=rtcp-fb:108 goog-remb\r\na=rtcp-fb:108 transport-cc\r\na=rtcp-fb:108 ccm fir\r\na=rtcp-fb:108 nack\r\na=rtcp-fb:108 nack pli\r\na=fmtp:108 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=42e01f\r\na=rtpmap:109 rtx/90000\r\na=fmtp:109 apt=108\r\na=rtpmap:124 red/90000\r\na=rtpmap:120 rtx/90000\r\na=fmtp:120 apt=124\r\na=rtpmap:123 ulpfec/90000\r\na=ssrc-group:FID 4008269645 4107779015\r\na=ssrc:4008269645 cname:Wn97EKOJFPlcc9wm\r\na=ssrc:4008269645 msid:lRp1CXMUAO275BhiToZVq6jbSE1dXSLV9hLh d6a76d64-297a-4b35-b813-30fe03fc6370\r\na=ssrc:4008269645 mslabel:lRp1CXMUAO275BhiToZVq6jbSE1dXSLV9hLh\r\na=ssrc:4008269645 label:d6a76d64-297a-4b35-b813-30fe03fc6370\r\na=ssrc:4107779015 cname:Wn97EKOJFPlcc9wm\r\na=ssrc:4107779015 msid:lRp1CXMUAO275BhiToZVq6jbSE1dXSLV9hLh d6a76d64-297a-4b35-b813-30fe03fc6370\r\na=ssrc:4107779015 mslabel:lRp1CXMUAO275BhiToZVq6jbSE1dXSLV9hLh\r\na=ssrc:4107779015 label:d6a76d64-297a-4b35-b813-30fe03fc6370\r\n";
	j["body"] = body;
	j["jsep"] = jsep;*/

	body["request"] = "create";
	body["room"] = 1234;
	body["description"] = "Echo Test Room";
	j["body"] = body;

	reqBody = seeker::json::toJsonString(j);
	sTime = seeker::time::currentTime();
	I_LOG("plugin endpoint reqBody = {}", reqBody);
	res = client->Post(url.c_str(), aheader, reqBody, "application/json");
	//res = client->Get(url.c_str(), aheader);
	cTime = seeker::time::currentTime() - sTime;

	if (cTime >= 1000) {
		W_LOG("[XrController::joinMeeting] Send request To vmcu Server use {}ms", cTime);
	}
	if (!res) {
		E_LOG("[XrController::joinMeeting] Send request To vmcu Server failed, errCode={}", (int)res.error());
		return -1;
	}
	if (res->status != 200) {
		E_LOG("[XrController::joinMeeting] vmcu Server response failed, status={}", res->status);
		return -1;
	}

	I_LOG("plugin endpoint respone body={}", res->body);

	/*json rsp;
	seeker::json::fromJsonString(rsp, res->body);
	I_LOG("j.errcode:{}", vrsp["errCode"].dump());
	if (vrsp["errCode"] != 0) {
		E_LOG("[XrController::joinMeeting] vmcu Server response errCode={}, msg={}", vrsp["errCode"].dump(), vrsp["msg"].dump());
		return -2;
	}*/


	seeker::Logger::shutdown();
	return 0;
}