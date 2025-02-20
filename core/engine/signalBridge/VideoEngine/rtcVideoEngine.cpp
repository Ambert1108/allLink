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
		rtc::scoped_refptr<CapturerTrackSource> video_device = CapturerTrackSource::Create();
		video_track_ = peer_connection_factory_->CreateVideoTrack(video_device, "camera");
		video_track = video_track_;
		auto result_or_error = peer_connection_->AddTrack(video_track_, { "000" });

		if (!result_or_error.ok()) {
			E_LOG("Failed to add video track to PeerConnection:{}", result_or_error.error().message());
			return -1;
		}

		auto receivers = peer_connection_->GetReceivers();
		for (auto& c : receivers) {
			if (!c) {
				std::cerr << "No RTP sender available." << std::endl;
				continue;
			}
			I_LOG("current sender id {}", c->id());
			if (c->id() != "audio_label") {
				// 获取当前的 RTP 参数
				webrtc::RtpParameters parameters = c->GetParameters();
				// 遍历所有编码设置并请求关键帧
				for (auto& encoding : parameters.encodings) {
					encoding.max_framerate = 30;
				}

				// 设置修改后的参数
				c->SetParameters(parameters);
			}
		}

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

	bool RTCVideoEngine::GetSourceList(
		webrtc::DesktopCapturer::SourceList* sources) {
		std::unique_ptr<webrtc::DesktopCapturer> screen_capturer(
			//webrtc::DesktopCapturer::CreateWindowCapturer(CreateDesktopCaptureOptions()));
			webrtc::DesktopCapturer::CreateScreenCapturer(
				CreateDesktopCaptureOptions()));
		return screen_capturer->GetSourceList(sources);
	}

	void RTCVideoEngine::getScreenMap(std::map<int, std::string>& screenMap) {
		webrtc::DesktopCapturer::SourceList sources;
		if (GetSourceList(&sources)) {
			int i = 0;
			for (webrtc::DesktopCapturer::Source& source : sources) {
				I_LOG("screen: dis_id = {}, id ={}, title = {}", source.display_id, source.id, source.title);
				screenMap.emplace(source.id, source.title);
			}
		}
	}

	bool RTCVideoEngine::GetWinSourceList(
		webrtc::DesktopCapturer::SourceList* sources) {
		std::unique_ptr<webrtc::DesktopCapturer> screen_capturer(
			webrtc::DesktopCapturer::CreateWindowCapturer(
				CreateDesktopCaptureOptions()));
		//webrtc::DesktopCapturer::CreateScreenCapturer(
		//	CreateDesktopCaptureOptions()));
		return screen_capturer->GetSourceList(sources);
	}

	void RTCVideoEngine::getWinMap(std::map<int, std::string>& winMap) {
		webrtc::DesktopCapturer::SourceList sources;
		if (GetWinSourceList(&sources)) {
			int i = 0;
			for (webrtc::DesktopCapturer::Source& source : sources) {
				I_LOG("screen: dis_id = {}, id ={}, title = {}", source.display_id, source.id, source.title);
				winMap.emplace(source.id, source.title);
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

		window_device = rtc::make_ref_counted<ScreenCapturer>();
		window_device->startWindowCapturer();

		if (screen_device) {
			screen_device->startCapturer();
			screen_track_ = peer_connection_factory_->CreateVideoTrack(screen_device, "screen");

			video_track = screen_track_;
			auto result_or_error = peer_connection_->AddTrack(screen_track_, { "111" });
			I_LOG("[VideoEngine::addScreenTrack] add track done");
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
		if (isScreenfirst) {
			std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders = peer_connection_->GetSenders();
			rtc::scoped_refptr<webrtc::RtpSenderInterface> sender = nullptr;
			for (const auto& c : senders) {
				I_LOG("id {}", c->track()->id());
				if (c->track()->id() == "screen") {
					sender = c;
					break;
				}
			}
			if (!sender) {
				E_LOG("find camera sender failed");
				return;
			}
			screen_track_ = peer_connection_factory_->CreateVideoTrack(screen_device, "screen");
			auto* track = reinterpret_cast<webrtc::MediaStreamTrackInterface*>(screen_track_.get());
			sender->SetTrack(track);
			isWinfirst = false;
			isWinfirst = true;
		}
		screen_device->setScreen(id);
	}

	void RTCVideoEngine::setWindowCapture(int id) {
		if (isWinfirst) {
			std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders = peer_connection_->GetSenders();
			rtc::scoped_refptr<webrtc::RtpSenderInterface> sender = nullptr;
			for (const auto& c : senders) {
				I_LOG("id {}", c->track()->id());
				if (c->track()->id() == "screen") {
					sender = c;
					break;
				}
			}
			if (!sender) {
				E_LOG("find camera sender failed");
				return;
			}
			screen_track_ = peer_connection_factory_->CreateVideoTrack(window_device, "screen");
			auto* track = reinterpret_cast<webrtc::MediaStreamTrackInterface*>(screen_track_.get());
			sender->SetTrack(track);
			isWinfirst = false;
			isScreenfirst = true;
		}
		window_device->setWindow(id);
	}

	int RTCVideoEngine::switchScreen(bool flag) {
		screen_track_->set_enabled(flag);
		screenState = flag;
		return 0;
	};

	bool RTCVideoEngine::getScreenState() {
		return screenState;
	}

	void RTCVideoEngine::setVideoBitrate(float bitrateKbps) {
		std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders = peer_connection_->GetSenders();
		for (auto& c : senders) {
			if (!c) {
				E_LOG("find sender is nullptr");
				continue;
			}
			if (c->id() == "camera") {
				I_LOG("current sender id {}", c->id());
				webrtc::RtpParameters parameters = c->GetParameters();
				for (auto& encoding : parameters.encodings) {
					encoding.max_bitrate_bps = bitrateKbps * 1000 * 1000;
					encoding.max_framerate = 30;
				}
				c->SetParameters(parameters);
			}
		}
	}

	void RTCVideoEngine::getVideoFactory(int mode, std::unique_ptr<webrtc::VideoEncoderFactory>& video_encoder_factory, 
		std::unique_ptr<webrtc::VideoDecoderFactory>& video_decoder_factory) {

		if (mode == 0) {
			video_encoder_factory = std::make_unique<webrtc::VideoEncoderFactoryTemplate<
				webrtc::OpenH264EncoderTemplateAdapter>>();
			video_decoder_factory = std::make_unique<webrtc::VideoDecoderFactoryTemplate<
				webrtc::OpenH264DecoderTemplateAdapter>>();
		}

		if (mode == 1) {
			video_encoder_factory = std::make_unique<webrtc::VideoEncoderFactoryTemplate<
				webrtc::LibvpxVp9EncoderTemplateAdapter>>();
			video_decoder_factory = std::make_unique<webrtc::VideoDecoderFactoryTemplate<
				webrtc::LibvpxVp9DecoderTemplateAdapter>>();
		}

		if (mode == 2) {
			video_encoder_factory = std::make_unique<webrtc::VideoEncoderFactoryTemplate<
				webrtc::LibvpxVp9EncoderTemplateAdapter,
				webrtc::OpenH264EncoderTemplateAdapter>>();
			video_decoder_factory = std::make_unique<webrtc::VideoDecoderFactoryTemplate<
				webrtc::LibvpxVp9DecoderTemplateAdapter,
				webrtc::OpenH264DecoderTemplateAdapter>>();
		}
	}		

	void RTCVideoEngine::close() {
		peer_connection_factory_ = nullptr;
		peer_connection_ = nullptr;
		video_track_ = nullptr;
		screen_track_ = nullptr;
		cameraDevice = nullptr;
		video_device1 = nullptr;
		if (screen_device) {
			screen_device->stopCapturer();
			screen_device = nullptr;
		}
		if (window_device) {
			window_device->stopCapturer();
			window_device = nullptr;
		}
		videoDevList.clear();
	}

}