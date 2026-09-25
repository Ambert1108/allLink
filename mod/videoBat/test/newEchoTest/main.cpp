#include<iostream>
#include "conductor.h"
#include "seeker/logger.h"
#include<seeker/loggerApi.h>
#include<nlohmann/json.hpp>
#include<nlohmann/fifomap.hpp>
#include "defaults.h"
#include"httplib.h"
#include "main_wnd.h"
template<class K, class V, class dummy_compare, class A>
using my_workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<my_workaround_fifo_map>;

httplib::Client client{ "10.1.29.246", 8088 };
namespace {

	std::string URL = "/janus";
	std::string transaction="test";
	long long createDataId;
	long long attachDataId;

	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
		peer_connection_factory_;

	//const std::string kAudioLabel= "audio_label";
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
	//const std::string kStreamId = "stream_id";
	std::string sdp;
}

int main() {
	seeker::Logger::init("test.log", false, true, true, "");

	if (SDL_Init(SDL_INIT_VIDEO))  // 初始化SDL视频模块
	{
		E_LOG("SDL_Init fuc fail");
	}


	std::shared_ptr< httplib::Client> cli = std::make_shared<httplib::Client>("10.1.29.246", 8088);

	MainWnd wnd("10.1.29.246", 8088,// 创建主窗口
		false, false);
	if (!wnd.Create()) {
		RTC_DCHECK_NOTREACHED();// 如果窗口创建失败，则终止程序
		return -1;
	}
	//PeerConnectionClient client;// 创建 PeerConnectionClient 对象
	auto conductor = rtc::make_ref_counted<Conductor>(&wnd);
	conductor->createSession();
	while (1) {
		std::this_thread::sleep_for(std::chrono::minutes(1));
	}
	system("pause");
	return 0;
}