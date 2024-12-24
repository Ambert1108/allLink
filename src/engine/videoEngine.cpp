#include "videoEngine.h"

rtc::scoped_refptr<CapturerTrackSource> CapturerTrackSource::Create(int i) {
	const size_t kWidth = 640;
	const size_t kHeight = 480;
	const size_t kFps = 30;
	std::unique_ptr<webrtc::test::VcmCapturer> capturer;
	std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
		webrtc::VideoCaptureFactory::CreateDeviceInfo());
	if (!info) {
		return nullptr;
	}
	int num_devices = info->NumberOfDevices();
	I_LOG("num_devices:{}", num_devices);
	I_LOG("1");
	capturer = absl::WrapUnique(
		webrtc::test::VcmCapturer::Create(kWidth, kHeight, kFps, i));
	I_LOG("2");
	if (capturer) {
		I_LOG("3");
		return rtc::make_ref_counted<CapturerTrackSource>(std::move(capturer));
		I_LOG("4");
	}
	/*for (int i = 0; i < num_devices; ++i) {
		I_LOG("1");
		capturer = absl::WrapUnique(
			webrtc::test::VcmCapturer::Create(kWidth, kHeight, kFps, i));
		I_LOG("2");
		if (capturer) {
			I_LOG("3");
			return rtc::make_ref_counted<CapturerTrackSource>(std::move(capturer));
			I_LOG("4");
		}
	}
	*/

	return nullptr;
}

rtc::VideoSourceInterface<webrtc::VideoFrame>* CapturerTrackSource::source() {
	return capturer_.get();
}

//class CapturerTrackSource : public webrtc::VideoTrackSource {
//public:
//	static rtc::scoped_refptr<CapturerTrackSource> Create() {
//		const size_t kWidth = 640;
//		const size_t kHeight = 480;
//		const size_t kFps = 30;
//		std::unique_ptr<webrtc::test::VcmCapturer> capturer;
//		std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
//			webrtc::VideoCaptureFactory::CreateDeviceInfo());
//		if (!info) {
//			return nullptr;
//		}
//		int num_devices = info->NumberOfDevices();
//		I_LOG("num_devices:{}", num_devices);
//		for (int i = 0; i < num_devices; ++i) {
//			I_LOG("1");
//			capturer = absl::WrapUnique(
//				webrtc::test::VcmCapturer::Create(kWidth, kHeight, kFps, i));
//			I_LOG("2");
//			if (capturer) {
//				I_LOG("3");
//				return rtc::make_ref_counted<CapturerTrackSource>(std::move(capturer));
//				I_LOG("4");
//			}
//		}
//
//		return nullptr;
//	}
//
//protected:
//	explicit CapturerTrackSource(
//		std::unique_ptr<webrtc::test::VcmCapturer> capturer)
//		: VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {
//	}
//
//private:
//	rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
//		return capturer_.get();
//	}
//	std::unique_ptr<webrtc::test::VcmCapturer> capturer_;
//};

VideoEngine::VideoEngine() {

}

VideoEngine::~VideoEngine() {
//RTC_DCHECK(!peer_connection_);
}

void VideoEngine::addVideoTrack(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>& peer_connection_factory,
	rtc::scoped_refptr<webrtc::PeerConnectionInterface>& peer_connection,
	rtc::scoped_refptr<webrtc::VideoTrackInterface>& video_track) {
	peer_connection_factory_ = peer_connection_factory;
	peer_connection_ = peer_connection;

	/*rtc::scoped_refptr<CapturerTrackSource> video_device =
		CapturerTrackSource::Create();*/
	video_device = CapturerTrackSource::Create(0);
	video_track_ = peer_connection_factory_->CreateVideoTrack(video_device, "video_label");
	video_track = video_track_;
	auto result_or_error = peer_connection_->AddTrack(video_track_, { "stream_id" });
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
	}
}

void VideoEngine::switchCamera(bool ifOpen) {
	video_track_->set_enabled(ifOpen);
	cameraState = ifOpen;
	//auto senders = peer_connection_->GetSenders();
	//for (auto& sender : senders) {
	//	if (sender->track() && sender->track()->kind() == webrtc::Med
	//	}iaStreamTrackInterface::kVideoKind) {
	//		sender->track()->set_enabled(ifOpen);
	//}
}

bool VideoEngine::getCameraState() {
	return cameraState;
}

int VideoEngine::getCameraNameMap(std::map<int, std::string>& cameraMap) {

}

int VideoEngine::setCameraDevice(const int index) {

}

void VideoEngine::close() {
	if(screen_device) screen_device->working = false;
	peer_connection_factory_ = nullptr;
	peer_connection_ = nullptr;
	video_track_ = nullptr;
	screen_track_ = nullptr;
}

void VideoEngine::removeTrack() {
	

	//auto senders = peer_connection_->GetSenders();
	//for (auto& sender : senders) {
	//	if (sender->track() && sender->track()->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
	//		sender->track()->set_enabled(ifOpen);
	//	}
	//}
	std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders = peer_connection_->GetSenders();
	rtc::scoped_refptr<webrtc::RtpSenderInterface> video_sender;

	for (const auto& sender : senders) {
		if (sender->track()->kind() == "video") {
			video_sender = sender;
			peer_connection_->RemoveTrackOrError(sender);
			break;
		}
	}
	if (!video_sender) {
		RTC_LOG(LS_WARNING) << "No video sender found!";
		return;
	}

}

void VideoEngine::switchTrack(rtc::scoped_refptr<webrtc::VideoTrackInterface>& new_video_track) {
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
	auto result_or_error = peer_connection_->AddTrack(new_video_track_, { "stream_id_new" });
	I_LOG("[VideoEngine::init] add new track done");

	//12/10
	//rtc::scoped_refptr<webrtc::VideoCaptureModule> vcm = webrtc::VideoCaptureFactory::Create("");
	//video_track_ = peer_connection_factory_->CreateVideoTrack(vcm, "");
	//

	//video_device =
	//	CapturerTrackSource::Create();
	//I_LOG("aa");
	//if (!video_device) 
	//	I_LOG("nullptr");
	//else {
	//	I_LOG("!nullptr");
	//}

	//new_video_track_
	//	= peer_connection_factory_->CreateVideoTrack(video_device, "video_label_new");
	//I_LOG("bb");

	//new_video_track = new_video_track_;
	//I_LOG("cc");
	//auto result_or_error = peer_connection_->AddTrack(new_video_track_, { "stream_id_new" });
	//I_LOG("[VideoEngine::init] add new track done");

	//for (const auto& sender : senders) {
	//	if (sender->track()->kind() == "video") {
	//		sender->SetTrack(new_video_track_);
	//		video_sender = sender;
	//		sender->track()->Release();
	//		//peer_connection_factory_->
	//		sender->Release();
	//		peer_connection_->RemoveTrackOrError(sender);
	//		video_device->Release();
	//		video_device = nullptr;
	//		
	//		break;
	//	}
	//}
	//if (!video_sender) {
	//	RTC_LOG(LS_WARNING) << "No video sender found!";
	//	return;
	//}
	//I_LOG("[VideoEngine::init] remove track done");

}

void VideoEngine::addScreenTrack(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>& peer_connection_factory,
	rtc::scoped_refptr<webrtc::PeerConnectionInterface>& peer_connection,
	rtc::scoped_refptr<webrtc::VideoTrackInterface>& video_track) {
	
	peer_connection_factory_ = peer_connection_factory;
	peer_connection_ = peer_connection;

	screen_device = rtc::make_ref_counted<MyCapturer>();
	
	if (screen_device) {
		screen_device->startCapturer();
		screen_track_ = peer_connection_factory_->CreateVideoTrack(screen_device, "video_label");
		video_track = screen_track_;
		auto result_or_error = peer_connection_->AddTrack(screen_track_, { "stream_id" });
		I_LOG("[VideoEngine::init] add track done");
		if (!result_or_error.ok()) {
			RTC_LOG(LS_ERROR) << "Failed to add video track to PeerConnection: "
				<< result_or_error.error().message();
		}
		
		
	}
	else {
		RTC_LOG(LS_ERROR) << "OpenVideoCaptureDevice failed";
	}


	

	/*auto senders = peer_connection_->GetSenders();
	
	for (auto& sender : senders) {
		if (sender->track() && sender->track()->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
			
			webrtc::RtpParameters parameters = sender->GetParameters();
			webrtc::RtpEncodingParameters encoding_parameters;
			
	}*/

}