#include "rtcVideoEngine.h"
namespace rtcengine {
	class TrackSource : public webrtc::VideoTrackSource {
	public:
		static rtc::scoped_refptr<TrackSource> Create() {
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
			//info->GetDeviceName(0, "")
			I_LOG("num_devices:{}", num_devices);
			for (int i = 0; i < num_devices; ++i) {
				capturer = absl::WrapUnique(
					webrtc::test::VcmCapturer::Create(kWidth, kHeight, kFps, i));
				if (capturer) {
					return rtc::make_ref_counted<TrackSource>(std::move(capturer));
				}
			}

			return nullptr;
		}

	protected:
		explicit TrackSource(
			std::unique_ptr<webrtc::test::VcmCapturer> capturer)
			: VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {
		}

	private:
		rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
			return capturer_.get();
		}
		std::unique_ptr<webrtc::test::VcmCapturer> capturer_;
	};

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

		rtc::scoped_refptr<webrtc::RtpSenderInterface> video_sender =
			peer_connection->GetSenders().at(0);
		
		webrtc::RtpParameters param(video_sender->GetParameters());
		if (param.codecs.empty()) {
			I_LOG("param is empty");
			webrtc::RtpCodecParameters codec_params;
			codec_params.payload_type = 100;
			param.codecs.push_back(codec_params);
		}
		else param.codecs.at(0).payload_type = 100;
		video_sender->SetParameters(param);

		rtc::scoped_refptr<TrackSource> video_device = TrackSource::Create();
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

		return cameraMap;
	}

	int RTCVideoEngine::setCamera(const int index) {

		return 0;
	}

	void RTCVideoEngine::close() {
		peer_connection_factory_ = nullptr;
		peer_connection_ = nullptr;
		video_track_ = nullptr;
	}
}