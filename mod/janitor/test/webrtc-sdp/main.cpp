#include<iostream>
#include "conductor.h"
#include "json/json.h"
using namespace std;
int main() {
//	std::string message = R"({
//	"sdp" : "v=0\r\no=- 3051387437173568662 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=group:BUNDLE 0 1\r\na=extmap-allow-mixed\r\na=msid-semantic: WMS stream_id\r\nm=audio 9 UDP/TLS/RTP/SAVPF 111 63 9 102 0 8 13 110 126\r\nc=IN IP4 0.0.0.0\r\na=rtcp:9 IN IP4 0.0.0.0\r\na=ice-ufrag:Vn+v\r\na=ice-pwd:ePwrzOS9b5d89aff6BnPbcoL\r\na=ice-options:trickle\r\na=fingerprint:sha-256 80:35:C9:1D:12:1D:9A:1A:B4:9A:A4:6E:45:5A:9D:F4:CD:38:CB:1A:70:C8:7E:A9:8D:D9:FF:5E:15:63:7B:AB\r\na=setup:actpass\r\na=mid:0\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=sendrecv\r\na=msid:stream_id audio_label\r\na=rtcp-mux\r\na=rtcp-rsize\r\na=rtpmap:111 opus/48000/2\r\na=rtcp-fb:111 transport-cc\r\na=fmtp:111 minptime=10;useinbandfec=1\r\na=rtpmap:63 red/48000/2\r\na=fmtp:63 111/111\r\na=rtpmap:9 G722/8000\r\na=rtpmap:102 ILBC/8000\r\na=rtpmap:0 PCMU/8000\r\na=rtpmap:8 PCMA/8000\r\na=rtpmap:13 CN/8000\r\na=rtpmap:110 telephone-event/48000\r\na=rtpmap:126 telephone-event/8000\r\na=ssrc:1266835901 cname:IfWrZlLdSquvjTDG\r\na=ssrc:1266835901 msid:stream_id audio_label\r\nm=video 9 UDP/TLS/RTP/SAVPF 96 97 98 99 100 101 39 40 127 103 104\r\nc=IN IP4 0.0.0.0\r\na=rtcp:9 IN IP4 0.0.0.0\r\na=ice-ufrag:Vn+v\r\na=ice-pwd:ePwrzOS9b5d89aff6BnPbcoL\r\na=ice-options:trickle\r\na=fingerprint:sha-256 80:35:C9:1D:12:1D:9A:1A:B4:9A:A4:6E:45:5A:9D:F4:CD:38:CB:1A:70:C8:7E:A9:8D:D9:FF:5E:15:63:7B:AB\r\na=setup:actpass\r\na=mid:1\r\na=extmap:14 urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:13 urn:3gpp:video-orientation\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay\r\na=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type\r\na=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing\r\na=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\r\na=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\r\na=sendrecv\r\na=msid:stream_id video_label\r\na=rtcp-mux\r\na=rtcp-rsize\r\na=rtpmap:96 VP8/90000\r\na=rtcp-fb:96 goog-remb\r\na=rtcp-fb:96 transport-cc\r\na=rtcp-fb:96 ccm fir\r\na=rtcp-fb:96 nack\r\na=rtcp-fb:96 nack pli\r\na=rtpmap:97 rtx/90000\r\na=fmtp:97 apt=96\r\na=rtpmap:98 VP9/90000\r\na=rtcp-fb:98 goog-remb\r\na=rtcp-fb:98 transport-cc\r\na=rtcp-fb:98 ccm fir\r\na=rtcp-fb:98 nack\r\na=rtcp-fb:98 nack pli\r\na=fmtp:98 profile-id=0\r\na=rtpmap:99 rtx/90000\r\na=fmtp:99 apt=98\r\na=rtpmap:100 VP9/90000\r\na=rtcp-fb:100 goog-remb\r\na=rtcp-fb:100 transport-cc\r\na=rtcp-fb:100 ccm fir\r\na=rtcp-fb:100 nack\r\na=rtcp-fb:100 nack pli\r\na=fmtp:100 profile-id=2\r\na=rtpmap:101 rtx/90000\r\na=fmtp:101 apt=100\r\na=rtpmap:39 AV1/90000\r\na=rtcp-fb:39 goog-remb\r\na=rtcp-fb:39 transport-cc\r\na=rtcp-fb:39 ccm fir\r\na=rtcp-fb:39 nack\r\na=rtcp-fb:39 nack pli\r\na=fmtp:39 level-idx=5;profile=0;tier=0\r\na=rtpmap:40 rtx/90000\r\na=fmtp:40 apt=39\r\na=rtpmap:127 red/90000\r\na=rtpmap:103 rtx/90000\r\na=fmtp:103 apt=127\r\na=rtpmap:104 ulpfec/90000\r\na=ssrc-group:FID 3307997155 2497128360\r\na=ssrc:3307997155 cname:IfWrZlLdSquvjTDG\r\na=ssrc:3307997155 msid:stream_id video_label\r\na=ssrc:2497128360 cname:IfWrZlLdSquvjTDG\r\na=ssrc:2497128360 msid:stream_id video_label\r\n",
//	"type" : "offer-loopback"
//})";
//	Json::CharReaderBuilder factory;
//	std::unique_ptr<Json::CharReader> reader =
//		absl::WrapUnique(factory.newCharReader());
//	Json::Value jmessage;
//	if (!reader->parse(message.data(), message.data() + message.length(),
//		&jmessage, nullptr)) {
//		RTC_LOG(LS_WARNING) << "Received unknown message. " << message;
//		E_LOG("Received unknown message.");
//	}
//	else {
//		I_LOG("reader->parse suc");
//	}
	try
	{
		auto conductor = rtc::make_ref_counted<Conductor>();
		if (!conductor->InitializePeerConnection()) {
			E_LOG("InitializePeerConnection error");
		}
		else {
			I_LOG("InitializePeerConnection success");
			if (!conductor->ReinitializePeerConnectionForLoopback()) {
				E_LOG("re error");
			}
			else {
				I_LOG("re success");
			}
		}
	}
	catch (const std::exception& e)
	{
		E_LOG("error [{}]", e.what());
	}
	return 0;
}