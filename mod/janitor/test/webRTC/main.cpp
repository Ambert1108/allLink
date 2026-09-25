#include<iostream>
#include "conductor.h"
#include "defaults.h"
//#include"utils/httplib.h"
//template<class K, class V, class dummy_compare, class A>
//using my_workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
//using json = nlohmann::basic_json<my_workaround_fifo_map>;
//
//httplib::Client client{ "10.1.29.246", 8088 };
//namespace {
//
//	std::string URL = "/janus";
//	std::string transaction="test";
//	long long createDataId;
//	long long attachDataId;
//
//	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
//		peer_connection_factory_;
//
//	//const std::string kAudioLabel= "audio_label";
//	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
//	//const std::string kStreamId = "stream_id";
//	std::string sdp;
//}

int main() {
	I_LOG("**********  test start  **********");

	oatpp::base::Environment::init();
	{
		auto conductor = rtc::make_ref_counted<Conductor>();
		conductor->createSession();
	}

	Sleep(10000);

	//conductor->resetPlugin();


	I_LOG("**********  test over  **********");
	while (1) {
		std::this_thread::sleep_for(std::chrono::hours(1));
	}

	oatpp::base::Environment::destroy();
	return 0;
}