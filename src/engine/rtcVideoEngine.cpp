#include "rtcVideoEngine.h"
namespace rtcengine {
	RTCVideoEngine::RTCVideoEngine() {

	}

	RTCVideoEngine::~RTCVideoEngine() {
		//RTC_DCHECK(!peer_connection_);
	}

	int RTCVideoEngine::addVideoTrack(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>& peer_connection_factory,
		rtc::scoped_refptr<webrtc::PeerConnectionInterface>& peer_connection,
		rtc::scoped_refptr<webrtc::VideoTrackInterface>& video_track) {
		peer_connection_factory_ = peer_connection_factory;
		peer_connection_ = peer_connection;

		//auto res = videoDevList.try_emplace(0, CapturerTrackSource::Create());
		//if (!res.second) {
		//	E_LOG("emplace video track failed");
		//	return -1;
		//}
		//rtc::scoped_refptr<CapturerTrackSource> device = CapturerTrackSource::Create();
		rtc::scoped_refptr<CapturerTrackSource> video_device = CapturerTrackSource::Create();
		video_track_ = peer_connection_factory_->CreateVideoTrack(video_device, "camera");
		video_track = video_track_;
		auto result_or_error = peer_connection_->AddTrack(video_track_, { "000" });

		if (!result_or_error.ok()) {
			RTC_LOG(LS_ERROR) << "Failed to add video track to PeerConnection: "
				<< result_or_error.error().message();
			return -1;
		}
		videoDevList.emplace(0, std::move(video_device));
		return 0;
	}

	void RTCVideoEngine::requestKeyFrame() {
		auto senders = peer_connection_->GetSenders();
		for (auto& c : senders) {
			if (!c) {
				std::cerr << "No RTP sender available." << std::endl;
				continue;
			}
			// 获取当前的 RTP 参数
			webrtc::RtpParameters parameters = c->GetParameters();
			I_LOG("current sender id {}", c->id());
			if (c->id() == "screen") {
				// 遍历所有编码设置并请求关键帧
				for (auto& encoding : parameters.encodings) {
					encoding.request_key_frame = true; // 请求关键帧
				}

				// 设置修改后的参数
				c->SetParameters(parameters);
				break;
			}
		}

		screen_track_->RequestRefreshFrame();
	}


	int RTCVideoEngine::switchCamera(bool flag) {
		video_track_->set_enabled(flag);
		cameraState = flag;
		return 0;
	};

	bool RTCVideoEngine::getCameraState() {
		return cameraState;
	}

	int RTCVideoEngine::setCamera(const int index, rtc::scoped_refptr<webrtc::VideoTrackInterface>& video_track) {
		// 停止当前的视频轨道
		if (video_track_) {
			I_LOG("Stopping current video track.");
			video_track_->set_enabled(false);
			video_track_ = nullptr;
		}
		// 创建新的视频设备
		rtc::scoped_refptr<CapturerTrackSource> video_device = CapturerTrackSource::Create(index);
		if (!video_device) {
			I_LOG("Failed to create video device for index: {}", index);
			return -1;
		}

		// 创建视频轨道
		video_track_ = peer_connection_factory_->CreateVideoTrack(video_device, "video_label");
		if (!video_track_) {
			I_LOG("Failed to create video track.");
			return -1;
		}
		video_track = video_track_;

		// 添加视频轨道到 PeerConnection
		auto result_or_error = peer_connection_->AddTrack(video_track_, { "stream_id" });
		if (!result_or_error.ok()) {
			I_LOG("Failed to add video track to PeerConnection: ");
			return -1;
		}

		I_LOG("Successfully added video track to PeerConnection.");
		return 0;
	}

	void RTCVideoEngine::switchTrack(rtc::scoped_refptr<webrtc::VideoTrackInterface>& new_video_track, int index) {
		/*std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders = peer_connection_->GetSenders();
		rtc::scoped_refptr<webrtc::RtpSenderInterface> video_sender;
		if (video_device1 == nullptr) {
			video_device1 =
				CapturerTrackSource::Create(1);
			if (!video_device1)
				I_LOG("nullptr");
			else {
				I_LOG("!nullptr");
			}
		}

		I_LOG("index:{}", index);
			

		for (const auto& sender : senders) {
			if (sender->track()->kind() == "video") {

				if (index == 0) {
					new_video_track_
						= peer_connection_factory_->CreateVideoTrack(video_device0, "video_label_new");


					new_video_track = new_video_track_;
					auto* track = reinterpret_cast<webrtc::MediaStreamTrackInterface*>(new_video_track_.release());
					sender->SetTrack(track);
				}
				if (index == 1) {
					new_video_track_
						= peer_connection_factory_->CreateVideoTrack(video_device1, "video_label_new");


					new_video_track = new_video_track_;
					auto* track = reinterpret_cast<webrtc::MediaStreamTrackInterface*>(new_video_track_.release());
					sender->SetTrack(track);
				}

				break;
			}
		}*/
	}

	void RTCVideoEngine::setCamera(int id) {
		std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders = peer_connection_->GetSenders();
		rtc::scoped_refptr<webrtc::RtpSenderInterface> sender = nullptr;
		for (const auto& c : senders) {
			I_LOG("id {}", c->track()->id());
			if (c->track()->id() == "camera") {
				sender = c;
				break;
			}
		}
		if (!sender) {
			E_LOG("find camera sender failed");
			return;
		}

		auto it = videoDevList.find(id);
		if (it != videoDevList.end()) {
			video_track_ = peer_connection_factory_->CreateVideoTrack(it->second, "camera");
			auto* track = reinterpret_cast<webrtc::MediaStreamTrackInterface*>(video_track_.get());
			sender->SetTrack(track);
		}
		else {
			rtc::scoped_refptr<CapturerTrackSource> device = CapturerTrackSource::Create(id);
			if (!device) {
				E_LOG("create device {} failed", id);
				return;
			}
			video_track_ = peer_connection_factory_->CreateVideoTrack(device, "camera");
			auto* track = reinterpret_cast<webrtc::MediaStreamTrackInterface*>(video_track_.get());
			sender->SetTrack(track);
			videoDevList.emplace(id, device);
		}

	}

	void RTCVideoEngine::getScreenMap(std::map<int16_t, std::string>& screenMap) {
		webrtc::DesktopCapturer::SourceList sources;
		if (GetSourceList(&sources)) {
			int i = 0;
			for (webrtc::DesktopCapturer::Source& source : sources) {
				I_LOG("screen: dis_id = {}, id ={}, title = {}", source.display_id, source.id, source.title);
				screenMap.emplace(source.id, source.title);
			}
		}
	}

	void RTCVideoEngine::getCameraMap(std::map<int16_t, std::string>& cameraMap) {
		std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
			webrtc::VideoCaptureFactory::CreateDeviceInfo());
		if (!info) {
			return;
		}

		int num_devices = info->NumberOfDevices();
		I_LOG("num_devices:{}", num_devices);
		for (int i = 0; i < num_devices; ++i) {
			char devName[256] = { 0 };
			char uniqueName[256] = { 0 };
			if (info->GetDeviceName(i, devName, 256, uniqueName, 256) != -1)
			{
				std::string devNameStr = devName;
				std::string devNameUniq = uniqueName;
				I_LOG("devName = {}, uniqueName = {}", devName, uniqueName);
				cameraMap.emplace(i, devName);
			}
		}
	}

	bool RTCVideoEngine::GetSourceList(
		webrtc::DesktopCapturer::SourceList* sources) {
		std::unique_ptr<webrtc::DesktopCapturer> screen_capturer(
			//webrtc::DesktopCapturer::CreateWindowCapturer(CreateDesktopCaptureOptions()));
			webrtc::DesktopCapturer::CreateScreenCapturer(
				CreateDesktopCaptureOptions()));
		return screen_capturer->GetSourceList(sources);
	}

	webrtc::DesktopCaptureOptions
		RTCVideoEngine::CreateDesktopCaptureOptions() {
		webrtc::DesktopCaptureOptions options =
			webrtc::DesktopCaptureOptions::CreateDefault();

		options.set_allow_directx_capturer(true);

		return options;
	}

	void RTCVideoEngine::addScreenTrack(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>& peer_connection_factory,
		rtc::scoped_refptr<webrtc::PeerConnectionInterface>& peer_connection,
		rtc::scoped_refptr<webrtc::VideoTrackInterface>& video_track) {

		peer_connection_factory_ = peer_connection_factory;
		peer_connection_ = peer_connection;
		
		screen_device = rtc::make_ref_counted<ScreenCapturer>();
		//std::map<int16_t, std::string> screen_map;
		//getScreenMap(screen_map);
		if (screen_device) {
			screen_device->startCapturer();
			screen_track_ = peer_connection_factory_->CreateVideoTrack(screen_device, "screen");
			video_track = screen_track_;
			auto result_or_error = peer_connection_->AddTrack(screen_track_, { "111" });
			I_LOG("[VideoEngine::init] add track done");
			if (!result_or_error.ok()) {
				RTC_LOG(LS_ERROR) << "Failed to add video track to PeerConnection: "
					<< result_or_error.error().message();
			}
		}
		else {
			RTC_LOG(LS_ERROR) << "OpenVideoCaptureDevice failed";
		}
	}

	void RTCVideoEngine::setScreenCapture(uint8_t id) {
		screen_device->setScreen(id);
	}

	int RTCVideoEngine::switchScreen(bool flag) {
		screen_track_->set_enabled(flag);
		screenState = flag;
		return 0;
	};

	bool RTCVideoEngine::getScreenState() {
		return screenState;
	}

	void RTCVideoEngine::close() {
		peer_connection_factory_ = nullptr;
		peer_connection_ = nullptr;
		video_track_ = nullptr;
		screen_track_ = nullptr;
		cameraDevice = nullptr;
		video_device1 = nullptr;
		videoDevList.clear();
	}

}