#include <windows.h>
#include <shellapi.h>

#include <string>
#include <vector>

#include "absl/flags/parse.h"
//#include "conductor.h"
//#include "examples/peerconnection/client/flag_defs.h"
//#include "main_wnd.h"
//#include "peerconnection.h"
#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "api/create_peerconnection_factory.h"
#include "api/async_dns_resolver.h"
#include "api/task_queue/pending_task_safety_flag.h"
#include "rtc_base/net_helpers.h"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "rtc_base/checks.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/string_utils.h"  // For ToUtf8
#include "rtc_base/win32_socket_init.h"
#include "rtc_base/thread.h"
#include "system_wrappers/include/field_trial.h"
#include "test/field_trial.h"
#include "seeker/logger.h"
#include "seeker/loggerApi.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "nlohmann/fifomap.hpp"
#include "HttpUtil.hpp"
#include "iostream"

//class CustomSocketServer : public  rtc::PhysicalSocketServer {
//public:
//  bool Wait(webrtc::TimeDelta max_wait_duration, bool process_io) override {
//    if (!process_io)
//      return true;
//
//    return rtc::PhysicalSocketServer::Wait(webrtc::TimeDelta::Zero(), process_io);
//  }
//
//};
//template<class K, class V, class dummy_compare, class A>
//using json_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
//using json = nlohmann::basic_json<json_fifo_map>;
HttpUtil httpUtil;
std::string ip = "10.1.29.246";
int port = 8088;
long long createDataId;
long long attachDataId;
rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;

int httpCallCreate() {
	json j;
	j["janus"] = "create";
	j["transaction"] = "transaction";
	std::string rsp_body;
	std::string url = "/janus";
	rsp_body = httpUtil.httpPost(ip, port, url, j);

	I_LOG("httpCallCreate rsp_body {}", rsp_body);
	j = json::parse(rsp_body);
	if (j["janus"] != "success") {
		return -1;
	}
	createDataId = j["data"]["id"];
	return 0;
}

int httpCallAttach() {
	json j;
	j["janus"] = "attach";
	j["plugin"] = "janus.plugin.echotest";
	j["transaction"] = "transaction";
	std::string rsp_body;
	std::string url = "/janus/" + std::to_string(createDataId);
	rsp_body = httpUtil.httpPost(ip, port, url, j);
	I_LOG("httpCallAttch rsp_body {}", rsp_body);
	j = json::parse(rsp_body);
	if (j["janus"] != "success") {
		return -1;
	}
	attachDataId = j["data"]["id"];
	return 1;
}

int httpCallMessage() {
	json j;
	j["janus"] = "message";
	j["transaction"] = "transaction";
	json j1;
	j1["audio"] = true;
	j["body"] = j1;
	std::string rsp_body;
	std::string url = "/janus/" + std::to_string(createDataId) + "/" + std::to_string(attachDataId);
	rsp_body = httpUtil.httpPost(ip, port, url, j);
	I_LOG("httpCallMessage rsp_body {}", rsp_body);
	return 1;
}

class CustomPeerConnectionObserver : public webrtc::PeerConnectionObserver {
public:
	void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {
		std::cout << "Signaling state: " << new_state << std::endl;
	}

	void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override {
		std::cout << "ICE connection state: " << new_state << std::endl;
	}

	void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override {
		std::cout << "ICE gathering state: " << new_state << std::endl;
	}

	void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) override {}

	void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {
		std::cout << "Stream added: " << stream->id() << std::endl;
	}

	void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {
		std::cout << "Stream removed: " << stream->id() << std::endl;
	}
};

void handleSignaling() {
	httpCallCreate();
	httpCallAttach();
	httpCallMessage();
	// 实现信令逻辑
	// 1. 创建HTTP连接以与Janus通信
	// 2. 发送offer/answer SDP描述
	// 3. 接收并处理remote SDP描述
	// 4. 交换ICE候选者
	// 5. 处理连接状态变更等

	std::cout << "Handling signaling... (this is a placeholder)" << std::endl;
}

void setupMediaStream(webrtc::PeerConnectionInterface* peer_connection) {
	// 创建媒体流并添加到PeerConnection
	std::cout << "Setting up media stream... (this is a placeholder)" << std::endl;

	// webrtc::MediaStreamInterface创建和管理媒体流
	// rtc::scoped_refptr<webrtc::MediaStreamInterface> local_stream =
	//     peer_connection_factory->CreateLocalMediaStream("local_stream");
	// ... (添加音视频轨道到local_stream)
	// peer_connection->AddStream(local_stream);
}





int main() {
  rtc::WinsockInitializer winsock_init;

  //CustomSocketServer ss;
  //rtc::AutoSocketServerThread main_thread(&ss);

  //
  //rtc::InitializeSSL();
  //

  //main_thread.Start();

	
	int a;
	std::cin >> a;

	rtc::InitializeSSL();

	auto network_thread = rtc::Thread::CreateWithSocketServer().release();
	auto signaling_thread = rtc::Thread::Create().release();
	auto worker_thread = rtc::Thread::Create().release();

	network_thread->Start();
	signaling_thread->Start();
	worker_thread->Start();

	auto peer_connection_factory = webrtc::CreatePeerConnectionFactory(
		network_thread, worker_thread, signaling_thread, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

	if (!peer_connection_factory) {
		std::cerr << "Failed to create PeerConnectionFactory" << std::endl;
		return -1;
	}

	// 配置PeerConnection
	webrtc::PeerConnectionInterface::RTCConfiguration config;
	config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;

	// 创建PeerConnection实例 (rtc::Ref AddRef 报错)
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection =
		peer_connection_factory->CreatePeerConnection(
			config, nullptr, nullptr, new rtc::RefCountedObject<CustomPeerConnectionObserver>());

	if (!peer_connection) {
		std::cerr << "Failed to create PeerConnection" << std::endl;
		return -1;
	}

	//添加媒体流和信令处理 (格式类型报错)
	webrtc::SessionDescription* offer = webrtc::CreateSessionDescription(webrtc::SdpSemantics::kUnifiedPlan);
	peer_connection->CreateOffer(offer, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());

	// 清理资源
	peer_connection = nullptr;
	peer_connection_factory = nullptr;
	network_thread->Stop();
	signaling_thread->Stop();
	worker_thread->Stop();
	rtc::CleanupSSL();

  return 0;
}

