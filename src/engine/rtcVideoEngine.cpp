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
		I_LOG("[VideoEngine::init] add track done");

		////set frameRate...
		//rtc::scoped_refptr<webrtc::RtpSenderInterface> sender = peer_connection_->GetSenders().at(0);
		//webrtc::RtpParameters parameters = sender->GetParameters();
		//for (auto& encoding : parameters.encodings) {
		//	encoding.max_framerate = 60;
		//}
		//sender->SetParameters(parameters);

		if (!result_or_error.ok()) {
			RTC_LOG(LS_ERROR) << "Failed to add video track to PeerConnection: "
				<< result_or_error.error().message();
			return -1;
		}
		return 0;
	}

	int RTCVideoEngine::switchCamera(bool flag) {
		video_track_->set_enabled(flag);
		cameraState = flag;
		return 0;
	};

	bool RTCVideoEngine::getCameraState() {
		return cameraState;
	}

	std::map<int, std::string> RTCVideoEngine::getCameraMap() {
		std::map<int, std::string> cameraMap{};
		std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
			webrtc::VideoCaptureFactory::CreateDeviceInfo());
		if (!info) {
			return {};
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

		return cameraMap;
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

	void RTCVideoEngine::switchTrack(rtc::scoped_refptr<webrtc::VideoTrackInterface>& new_video_track) {
		std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders = peer_connection_->GetSenders();
		rtc::scoped_refptr<webrtc::RtpSenderInterface> video_sender;


		for (const auto& sender : senders) {
			if (sender->track()->kind() == "video") {
				video_sender = sender;
				sender->track()->Release();
				//ender->SetTrack()
				//peer_connection_factory_->

				sender->Release();
				peer_connection_->RemoveTrackOrError(sender);
				video_device->Release();
				video_device = nullptr;

				break;
			}
		}
		if (!video_sender) {
			RTC_LOG(LS_WARNING) << "No video sender found!";
			return;
		}
		I_LOG("[VideoEngine::init] remove track done");



		video_device =
			CapturerTrackSource::Create(1);
		I_LOG("aa");
		if (!video_device)
			I_LOG("nullptr");
		else {
			I_LOG("!nullptr");
		}

		new_video_track_
			= peer_connection_factory_->CreateVideoTrack(video_device, "video_label_new");
		I_LOG("bb");


		new_video_track = new_video_track_;
		I_LOG("cc");
		auto result_or_error = peer_connection_->AddTrack(new_video_track_, { "video_label" });
		I_LOG("[VideoEngine::init] add new track done");
	}

	const std::string RTCVideoEngine::GetSourceListString() {
		std::ostringstream oss;
		webrtc::DesktopCapturer::SourceList sources;
		if (GetSourceList(&sources)) {
			int i = 0;
			for (webrtc::DesktopCapturer::Source& source : sources) {
				oss << std::to_string(i++) << " : " << source.title << std::endl;
				I_LOG("screen[{}]: title = {}", i, source.title);
			}
		}
		return oss.str();
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
		GetSourceListString();
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
	}

}