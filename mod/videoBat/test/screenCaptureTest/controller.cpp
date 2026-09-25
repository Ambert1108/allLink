#include "controller.h"

#include <stddef.h>
#include <stdint.h>

#include "absl/memory/memory.h"
#include "api/audio/audio_device.h"
#include "api/audio/audio_mixer.h"
#include "api/audio/audio_processing.h"
#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/audio_options.h"
#include "api/create_peerconnection_factory.h"
#include "api/rtp_sender_interface.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_decoder_factory_template.h"
#include "api/video_codecs/video_decoder_factory_template_dav1d_adapter.h"
#include "api/video_codecs/video_decoder_factory_template_libvpx_vp8_adapter.h"
#include "api/video_codecs/video_decoder_factory_template_libvpx_vp9_adapter.h"
#include "api/video_codecs/video_decoder_factory_template_open_h264_adapter.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "api/video_codecs/video_encoder_factory_template.h"
#include "api/video_codecs/video_encoder_factory_template_libaom_av1_adapter.h"
#include "api/video_codecs/video_encoder_factory_template_libvpx_vp8_adapter.h"
#include "api/video_codecs/video_encoder_factory_template_libvpx_vp9_adapter.h"
#include "api/video_codecs/video_encoder_factory_template_open_h264_adapter.h"
#include "defaults.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "p2p/base/port_allocator.h"
#include "pc/video_track_source.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "rtc_base/strings/json.h"
#include "test/vcm_capturer.h"
#include <memory>
#include <optional>
#include <utility>
#include <vector>
namespace {
	// Names used for a IceCandidate JSON object.
	const char kCandidateSdpMidName[] = "sdpMid";
	const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
	const char kCandidateSdpName[] = "candidate";

	// Names used for a SessionDescription JSON object.
	const char kSessionDescriptionTypeName[] = "type";
	const char kSessionDescriptionSdpName[] = "sdp";

	class DummySetSessionDescriptionObserver
		: public webrtc::SetSessionDescriptionObserver {
	public:
		static rtc::scoped_refptr<DummySetSessionDescriptionObserver> Create() {
			return rtc::make_ref_counted<DummySetSessionDescriptionObserver>();
		}
		virtual void OnSuccess() { RTC_LOG(LS_INFO) << __FUNCTION__; }
		virtual void OnFailure(webrtc::RTCError error) {
			RTC_LOG(LS_INFO) << __FUNCTION__ << " " << ToString(error.type()) << ": "
				<< error.message();
		}
	};
	class CapturerTrackSource : public webrtc::VideoTrackSource {
	public:
		static rtc::scoped_refptr<CapturerTrackSource> Create() {
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
			for (int i = 0; i < num_devices; ++i) {
				capturer = absl::WrapUnique(
					webrtc::test::VcmCapturer::Create(kWidth, kHeight, kFps, i));
				if (capturer) {
					return rtc::make_ref_counted<CapturerTrackSource>(std::move(capturer));
					char name[256];
					uint32_t deviceNameLength = 100;
					char deviceUniqueIdUTF8[256];
					uint32_t deviceUniqueIdUTF8Length = 100;
					info->GetDeviceName(i, name, deviceNameLength, deviceUniqueIdUTF8, deviceUniqueIdUTF8Length);
					I_LOG("i:{} name:{} deviceNameLength:{} deviceUniqueIdUTF8:{} deviceUniqueIdUTF8Length:{}", i, name, deviceNameLength, deviceUniqueIdUTF8, deviceUniqueIdUTF8Length);
				}
			}

			return nullptr;
		}

	protected:
		explicit CapturerTrackSource(
			std::unique_ptr<webrtc::test::VcmCapturer> capturer)
			: VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {
		}

	private:
		rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
			return capturer_.get();
		}
		std::unique_ptr<webrtc::test::VcmCapturer> capturer_;
	};

	class MyCustomVideoEncoder : public webrtc::VideoEncoder {
	public:
		MyCustomVideoEncoder() {
			// 初始化编码器
		}

		// 实现编码器的必要方法
		int32_t InitEncode(const webrtc::VideoCodec* codec_settings, int32_t number_of_cores, size_t max_payload_size) override {
			// 初始化编码器设置
			// 检查 codec_settings 是否有效
			//if (!codec_settings) {
			//	return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
			//}

			//codec_settings->maxFramerate = 10;

			//// 从 codec_settings 中提取参数
			//target_bitrate_kbps_ = codec_settings->startBitrate; // 目标比特率（kbps）
			//framerate_ = codec_settings->maxFramerate; // 最大帧率
			//width_ = codec_settings->width; // 视频宽度
			//height_ = codec_settings->height; // 视频高度

			//// 根据 codec_settings 设置编码器的其他参数
			//// 例如，设置编码器的质量、GOP 大小等
			//if (codec_settings->codecType == kVideoCodecH264) {
			//	// H264 特定设置
			//}
			//else if (codec_settings->codecType == kVideoCodecVP8) {
			//	// VP8 特定设置
			//}

			// 如果需要，可以设置编码器的其他初始化参数
			// 比如，初始化内部状态、分配资源等
			return WEBRTC_VIDEO_CODEC_OK;
		}

		int32_t Encode(const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frame_types) override {
			//int32_t Encode(const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frame_types, const webrtc::CodecSpecificInfo* codec_specific_info, int64_t time_stamp) override {
				// 编码视频帧
			return WEBRTC_VIDEO_CODEC_OK;
		}

		int32_t RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback) override {
			// 注册编码完成回调
			return WEBRTC_VIDEO_CODEC_OK;
		}

		int32_t Release() override {
			// 释放资源
			return WEBRTC_VIDEO_CODEC_OK;
		}

		// 其他必要的实现...
		void SetRates(const webrtc::VideoEncoder::RateControlParameters& parameters) override {
			//设置码率
		}

		webrtc::VideoEncoder::EncoderInfo GetEncoderInfo() const override {
			return webrtc::VideoEncoder::EncoderInfo();

		}
	};

	class MyCustomVideoEncoderFactory : public webrtc::VideoEncoderFactory {
	public:
		MyCustomVideoEncoderFactory() {}

		// 返回支持的编码格式
		std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override {
			return { webrtc::SdpVideoFormat("H264"), webrtc::SdpVideoFormat("VP8") }; // 示例支持 H264 和 VP8
		}

		// 创建编码器实例
		std::unique_ptr<webrtc::VideoEncoder> Create(const webrtc::Environment& env, const webrtc::SdpVideoFormat& format) override {
			if (format.name == "H264") {
				//    //return rtc::make_ref_counted<MyCustomVideoEncoder>();
				return std::make_unique<MyCustomVideoEncoder>();
			}
			// 处理其他格式...
			return nullptr;
		}

		// 其他必要的实现...
		
		void Release() {
			return;
		}

	};

}
Conductor::Conductor(std::shared_ptr< httplib::Client> _cli) {
	loopback_ = false;
	cli = _cli;

	/*wnd = new sf::RenderWindow(sf::VideoMode(sf::VideoMode::getDesktopMode().width / 2,
		sf::VideoMode::getDesktopMode().width / 2), "local Capture", sf::Style::Close);*/
	wnd = new sf::RenderWindow(sf::VideoMode(sf::VideoMode::getDesktopMode().width / 2,
		sf::VideoMode::getDesktopMode().height), "local Capture", sf::Style::Close);

	wnd->setFramerateLimit(60);
	localSource = new sf::Texture();
	remoteSource = new sf::Texture();
	screenSource = new sf::Texture();
	wndWidth = wnd->getSize().x;
	wndHeight = wnd->getSize().y;
	//create();
	//attach();
	//deattach();

	ve = std::make_unique<rtcengine::RTCVideoEngine>();
	std::map<int, std::string> cameraMap{};
	I_LOG("111");
	//ve->getCameraNameMap(cameraMap);
	//for (auto& i : cameraMap) {
	//
	//}
}
Conductor::~Conductor() {
	RTC_DCHECK(!peer_connection_);
}

bool Conductor::InitializePeerConnection()
{
	RTC_DCHECK(!peer_connection_factory_);
	RTC_DCHECK(!peer_connection_);

	if (!signaling_thread_.get()) {
		signaling_thread_ = rtc::Thread::CreateWithSocketServer();
		signaling_thread_->Start();
	}

	//test
	// 创建自定义编码器工厂
	//rtc::scoped_refptr<webrtc::VideoEncoderFactory> my_encoder_factory = rtc::make_ref_counted<MyCustomVideoEncoderFactory>();
	//std::unique_ptr<webrtc::VideoEncoderFactory> my_encoder_factory = std::make_unique<MyCustomVideoEncoderFactory>();
	//std::unique_ptr<webrtc::VideoEncoderFactory> my_f = std::make_unique<webrtc::VideoEncoderFactory>();
	//my_f->

	std::unique_ptr<webrtc::VideoEncoderFactory> video_encoder_factory = nullptr;
	std::unique_ptr<webrtc::VideoDecoderFactory> video_decoder_factory = nullptr;
	ve->getVideoFactory(1, video_encoder_factory, video_decoder_factory);

		peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
			nullptr /* network_thread */, nullptr /* worker_thread */,
			signaling_thread_.get(), nullptr /* default_adm */,
			webrtc::CreateBuiltinAudioEncoderFactory(),
			webrtc::CreateBuiltinAudioDecoderFactory(),
			std::move(video_encoder_factory),
			std::move(video_decoder_factory),
			nullptr /* audio_mixer */, nullptr /* audio_processing */);
		


	if (!peer_connection_factory_) {
		DeletePeerConnection();
		return false;
	}

	if (!CreatePeerConnection()) {
		DeletePeerConnection();
	}

	AddTracks();

	return peer_connection_ != nullptr;
}

bool Conductor::ReinitializePeerConnectionForLoopback()
{
	I_LOG("a");
	loopback_ = true;
	std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders =
		peer_connection_->GetSenders();
	peer_connection_ = nullptr;
	I_LOG("b");
	// Loopback is only possible if encryption is disabled.
	webrtc::PeerConnectionFactoryInterface::Options options;
	options.disable_encryption = true;
	peer_connection_factory_->SetOptions(options);
	I_LOG("c");
	if (CreatePeerConnection()) {
		for (const auto& sender : senders) {
			peer_connection_->AddTrack(sender->track(), sender->stream_ids());
		}
		peer_connection_->CreateOffer(
			this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
	}
	I_LOG("d");
	options.disable_encryption = false;
	peer_connection_factory_->SetOptions(options);
	I_LOG("e");
	return peer_connection_ != nullptr;
}

bool Conductor::CreatePeerConnection()
{
	RTC_DCHECK(peer_connection_factory_);
	RTC_DCHECK(!peer_connection_);

	webrtc::PeerConnectionInterface::RTCConfiguration config;
	config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
	webrtc::PeerConnectionInterface::IceServer server;
	server.uri = GetPeerConnectionString();
	config.servers.push_back(server);
	webrtc::PeerConnectionDependencies pc_dependencies(this);
	auto error_or_peer_connection =
		peer_connection_factory_->CreatePeerConnectionOrError(
			config, std::move(pc_dependencies));
	if (error_or_peer_connection.ok()) {
		peer_connection_ = std::move(error_or_peer_connection.value());
	}
	return peer_connection_ != nullptr;
}

void Conductor::DeletePeerConnection()
{
	peer_connection_ = nullptr;
	peer_connection_factory_ = nullptr;
	peer_id_ = -1;
	loopback_ = false;
}

void Conductor::EnsureStreamingUI()
{
	RTC_DCHECK(peer_connection_);
}

void Conductor::AddTracks()
{
	rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_ = nullptr;
	rtc::scoped_refptr<webrtc::VideoTrackInterface> screen_track_ = nullptr;
	
	ve->addVideoTrack(peer_connection_factory_, peer_connection_, video_track_);
	ve->addScreenTrack(peer_connection_factory_, peer_connection_, screen_track_);
	
	local_renderer_.reset(new VideoRenderer(std::bind(&Conductor::OnPaint, this), 1, 1, video_track_.get()));
}

int64_t poll_print(int64_t& origin_time, int64_t poll_time) {
	int64_t current_time = seeker::Time::currentTime();
	int64_t	diff_time = current_time - origin_time;
	if (diff_time >= poll_time) {
		origin_time = current_time;
		return diff_time;
	}
	return -1;
}

void Conductor::start()
{
	try {
		create();
		I_LOG("11111 create done");
		attach();
		sendmessage();
		I_LOG("11111 sendMsg done");
		if (InitializePeerConnection()) {
			peer_connection_->CreateOffer(
				this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
		}
		else {
			E_LOG("InitializePeerConnection error");
		}
		std::thread lk(&Conductor::keeplive, this);
		lk.detach();
		//while (true) {
		//	if (answersdp == "") {
		//		//getJanu();
		//	}
		//	else {
		//		break;
		//	}
		//}
		auto poll_origin_time = seeker::Time::currentTime();
		ve->startStatsCollection();
		while (wnd->isOpen()) {
			//I_LOG("111");
			double outFps = 0.0, inFps = 0.0;
			double outBitrate = 0.0, inBitrate = 0.0;
			ve->getCurrentFrameRate(outFps, inFps);
			ve->getCurrentBitrate(outBitrate, inBitrate);
			if (poll_print(poll_origin_time, 1000) >= 0) {
				I_LOG("outFps={},inFps={},outbt={},intbt={}", outFps, inFps, outBitrate, inBitrate);
			}
			while (wnd->pollEvent(event)) {
				switch (event.type) {
				case sf::Event::Closed:
					wnd->close();
					break;

				case sf::Event::KeyPressed:
					if (event.key.code == sf::Keyboard::Escape) {
						wnd->close();
					}
					else if (event.key.code == sf::Keyboard::M) {
						//isMirror.store(!isMirror);
					}
					else if (event.key.code == sf::Keyboard::O) {
						ve->switchCamera(true);
						ve->switchScreen(true);
						
						//switchCamera(true);
					}
					else if (event.key.code == sf::Keyboard::C) {
						ve->switchCamera(false);
						ve->switchScreen(false);
						ve->stopStatsCollection();
						//switchCamera(false);
					}
					else if (event.key.code == sf::Keyboard::R) {
						std::map<int16_t, std::string> videoInputDevMap;
						ve->getCameraMap(videoInputDevMap);
						for (auto it = videoInputDevMap.begin(); it != videoInputDevMap.end(); ++it) {
							I_LOG("Device ID:={},Name:={}", it->first, it->second);
						}
						replaceTrack(0);
					}
					else if (event.key.code == sf::Keyboard::F) {
						replaceTrack(1);

					}
					else if (event.key.code == sf::Keyboard::S) {
						ve->setScreenCapture(0);
					}
					else if (event.key.code == sf::Keyboard::W) {
						ve->setScreenCapture(1);
					}
					else if (event.key.code == sf::Keyboard::I) {
						std::map<int, std::string> winMap;
						ve->getWinMap(winMap);
						for (auto it = winMap.begin(); it != winMap.end(); ++it) {
							I_LOG("Device ID:={},Name:={}", it->first, it->second);
						}
						ve->setWindowCapture(appNum);
						if (appNum < winMap.size() - 1)
	            appNum++;
	          else appNum = 1;
					}
					else if (event.key.code == sf::Keyboard::X) {
						rtc::scoped_refptr<webrtc::VideoTrackInterface> screen_track_ = nullptr;

						ve->close();
						ve->addScreenTrack(peer_connection_factory_, peer_connection_, screen_track_);
						ve->setScreenCapture(0);
					}
					I_LOG("camera:{}", ve->getCameraState());
					break;
				}
			}

			wnd->clear(sf::Color(128, 128, 128));
			ImageData localData;
			ImageData remoteData;
			ImageData screenData;
			//I_LOG("localImageList size:{}", localImageList.Size());
			//I_LOG("remoteImageList size:{}", remoteImageList.Size());
			if (localImageList.TryPopFlex(localData)) {
				int localHeight = abs(localData.bmi.bmiHeader.biHeight);
				int localWidth = localData.bmi.bmiHeader.biWidth;
				bool reset = false;
				if (localSource->getSize().x != localWidth
					|| localSource->getSize().y != localHeight) {
					D_LOG("remote size is {}:{}, raw size is {}:{}", localSource->getSize().x, localSource->getSize().y,
						localWidth, localHeight);
					localSource->create(localWidth, localHeight);
					reset = true;
				}
				localSource->update(localData.image.get());


				//I_LOG("loacalSource w:{} h:{}", localSource->getSize().x, localSource->getSize().y);
				localVideo.setTexture(*localSource, reset);

				//I_LOG("localVideo w:{} h:{}", localVideo.getGlobalBounds().width, localVideo.getGlobalBounds().height);
				int localX = (wndWidth - localWidth) / 2;
				int localY = (wndHeight - localHeight) / 2;
				//localVideo.setPosition(sf::Vector2f(localX, localY));
				localVideo.setPosition(sf::Vector2f(0, 0));
				wnd->draw(localVideo);
			}
			if (remoteImageList.TryPopFlex(remoteData)) {
				int remoteHeight = abs(remoteData.bmi.bmiHeader.biHeight);
				int remoteWidth = remoteData.bmi.bmiHeader.biWidth;
				bool reset = false;
				if (remoteSource->getSize().x != remoteWidth
					|| remoteSource->getSize().y != remoteHeight) {
					D_LOG("remote size is {}:{}, raw size is {}:{}", remoteSource->getSize().x, remoteSource->getSize().y,
						remoteWidth, remoteHeight);
					remoteSource->create(remoteWidth, remoteHeight);
					reset = true;
				}
				remoteSource->update(remoteData.image.get());
				//I_LOG("remoteSource w:{} h:{}", remoteSource->getSize().x, remoteSource->getSize().y);
				remoteVideo.setTexture(*remoteSource, reset);
				int localX = (wndWidth - remoteWidth) / 2;
				int localY = (wndHeight - remoteHeight) / 2;
				//localVideo.setPosition(sf::Vector2f(localX, localY));
				remoteVideo.setPosition(sf::Vector2f(wnd->getSize().x / 2, 0));
				wnd->draw(remoteVideo);
			}
			if (screenImageList.TryPopFlex(screenData)) {
				int screenHeight = abs(screenData.bmi.bmiHeader.biHeight);
				int screenWidth = screenData.bmi.bmiHeader.biWidth;
				bool reset = false;
				if (screenSource->getSize().x != screenWidth
					|| screenSource->getSize().y != screenHeight) {
					D_LOG("screen size is {}:{}, raw size is {}:{}", screenSource->getSize().x, screenSource->getSize().y,
						screenWidth, screenHeight);
					screenSource->create(screenWidth, screenHeight);
					reset = true;
				}
				screenSource->update(screenData.image.get());
				//I_LOG("remoteSource w:{} h:{}", remoteSource->getSize().x, remoteSource->getSize().y);
				screenVideo.setTexture(*screenSource, reset);
				int screenX = (wndWidth - screenWidth) / 2;
				int screenY = (wndHeight - screenHeight) / 2;
				//localVideo.setPosition(sf::Vector2f(localX, localY));
				screenVideo.setPosition(sf::Vector2f(0, wnd->getSize().y / 3));
				wnd->draw(screenVideo);
			}
			wnd->display();
		}
	}
	catch (const std::exception& ex) {
		E_LOG("{}", ex.what());
	}
	catch (...) {
		E_LOG("Unknown exception caught");
	}
}

void Conductor::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams)
{
	I_LOG("on add track, id:{}", receiver->id());
	RTC_LOG(LS_INFO) << __FUNCTION__ << " " << receiver->id();
	I_LOG("a");
	auto* track = reinterpret_cast<webrtc::MediaStreamTrackInterface*>(receiver->track().release());
	I_LOG("b");
	if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) I_LOG("aaa");
	auto* video_track = static_cast<webrtc::VideoTrackInterface*>(track);
	I_LOG("c");
	if (!remote_renderer_) {
		remote_renderer_.reset(new VideoRenderer(std::bind(&Conductor::OnPaint, this), 1, 1, video_track));
	}
	else {
		screen_renderer_.reset(new VideoRenderer(std::bind(&Conductor::OnPaint, this), 1, 1, video_track));
	}
	track->Release();
	I_LOG("d");
}

void Conductor::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transer) {
	I_LOG("aaaaaaaaaaaaaaaaaaaaaa");
	return;
}

void Conductor::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)
{
	I_LOG("OnRemoveTrack");
	RTC_LOG(LS_INFO) << __FUNCTION__ << " " << receiver->id();
}
void Conductor::OnIceGatheringChange(
	webrtc::PeerConnectionInterface::IceGatheringState new_state) {
	if (new_state == webrtc::PeerConnectionInterface::kIceGatheringComplete) {
		I_LOG("kIceGatheringComplete");
		sendtrickle();
	}
}
void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
{
	RTC_LOG(LS_INFO) << __FUNCTION__ << " " << candidate->sdp_mline_index();
	// For loopback test. To save some connecting delay.
	if (loopback_) {
		if (!peer_connection_->AddIceCandidate(candidate)) {
			RTC_LOG(LS_WARNING) << "Failed to apply the received candidate";
		}
		return;
	}
	std::string sdp;
	if (!candidate->ToString(&sdp)) {
		RTC_LOG(LS_ERROR) << "Failed to serialize candidate";
		return;
	}
	sendtrickle(sdp, candidate->sdp_mid(), candidate->sdp_mline_index());
}

void Conductor::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
	I_LOG("onSuccess");
	peer_connection_->SetLocalDescription(
		DummySetSessionDescriptionObserver::Create().get(), desc);

	std::string sdp;
	desc->ToString(&sdp);
	I_LOG("SDP [{}]", sdp);
	// For loopback test. To save some connecting delay.
	if (loopback_) {
		// Replace message type from "offer" to "answer"
		std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
			webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, sdp);
		I_LOG("!!!!!!!!!!!!!!11");
		peer_connection_->SetRemoteDescription(
			DummySetSessionDescriptionObserver::Create().get(),
			session_description.release());
		return;
	}
	sendoffersdp(sdp);
}

void Conductor::OnFailure(webrtc::RTCError error)
{
	RTC_LOG(LS_ERROR) << ToString(error.type()) << ": " << error.message();
}

bool Conductor::create()
{
	json create_session_request;
	create_session_request["janus"] = "create";
	std::string transaction_id = seeker::secure::randomChars(8);
	create_session_request["transaction"] = transaction_id;
	auto res = cli->Post("/janus_restful/", create_session_request.dump(), "application/json");
	if (res == nullptr) {
		E_LOG("janus link error");
		return false;
	}
	else {
		if (res->status == 200) {
			json response = json::parse(res->body);
			I_LOG("janus create resp [{}]", response.dump());
			session_id = response["data"]["id"];
			return true;
		}
		else {
			E_LOG("janus create status error [{}]", res->status);
			return false;
		}
	}
}

bool Conductor::attach()
{
	// ���ӵ�EchoTest���
	json create_plug_request;
	create_plug_request["janus"] = "attach";
	std::string transaction_id = seeker::secure::randomChars(8);
	create_plug_request["transaction"] = transaction_id;
	create_plug_request["plugin"] = "janus.plugin.echotest";
	auto res = cli->Post("/janus_restful/" + std::to_string(session_id), create_plug_request.dump(), "application/json");
	if (res == nullptr) {
		E_LOG("janus session error");
		return false;
	}
	else {
		if (res->status == 200) {
			json response = json::parse(res->body);
			I_LOG("janus session resp [{}]", response.dump());
			handle_id = response["data"]["id"];
			return true;
		}
		else {
			E_LOG("janus plug status error [{}]", res->status);
			return false;
		}
	}
}

bool Conductor::sendmessage()
{
	json config_request;
	config_request["janus"] = "message";
	std::string transaction_id = seeker::secure::randomChars(8);
	config_request["transaction"] = transaction_id;
	config_request["body"]["audio"] = false;
	config_request["body"]["video"] = true;
	auto res = cli->Post("/janus_restful/" + std::to_string(session_id) + "/" + std::to_string(handle_id), config_request.dump(), "application/json");
	if (res == nullptr) {
		E_LOG("janus handle link error");
		return false;
	}
	else {
		if (res->status == 200) {
			json response = json::parse(res->body);
			I_LOG("janus handle resp [{}]", response.dump());
			//getJanus(transaction_id, false);
			getJanu();
			return true;
		}
		else {
			E_LOG("janus handle status error [{}]", res->status);
			return false;
		}
	}
}

bool Conductor::sendoffersdp(std::string offersdp)
{
	json config_request;
	config_request["janus"] = "message";
	std::string transaction_id = seeker::secure::randomChars(8);
	config_request["transaction"] = transaction_id;
	config_request["body"]["audio"] = false;
	config_request["body"]["video"] = true;
	config_request["jsep"]["type"] = "offer";
	config_request["jsep"]["sdp"] = offersdp;
	auto res = cli->Post("/janus_restful/" + std::to_string(session_id) + "/" + std::to_string(handle_id), config_request.dump(), "application/json");
	I_LOG("send offer json:{}", config_request.dump());
	if (res == nullptr) {
		E_LOG("janus handle link error");
		return false;
	}
	else {
		if (res->status == 200) {
			json response = json::parse(res->body);
			I_LOG("janus handle resp [{}]", response.dump());

			//I_LOG("janus handle resp [{}]", response.dump());
			getJanu();
			//return true;

			offid = transaction_id;
			return true;
		}
		else {
			E_LOG("janus handle status error [{}]", res->status);
			return false;
		}
	}
}

bool Conductor::sendtrickle(std::string candidate, std::string sdpMid, int sdpMLineIndex)
{
	json config_request;
	config_request["janus"] = "trickle";
	std::string transaction_id = seeker::secure::randomChars(8);
	config_request["transaction"] = transaction_id;
	config_request["candidate"]["candidate"] = candidate;
	config_request["candidate"]["sdpMLineIndex"] = sdpMLineIndex;
	config_request["candidate"]["sdpMid"] = sdpMid;
	I_LOG("send trickle:{}", config_request.dump());
	auto res = cli->Post("/janus_restful/" + std::to_string(session_id) + "/" + std::to_string(handle_id), config_request.dump(), "application/json");
	if (res == nullptr) {
		E_LOG("janus handle link error");
		return false;
	}
	else {
		if (res->status == 200) {
			json response = json::parse(res->body);
			I_LOG("janus handle resp [{}]", response.dump());
			return true;
		}
		else {
			E_LOG("janus handle status error [{}]", res->status);
			return false;
		}
	}
}

bool Conductor::sendtrickle()
{
	json config_request;
	config_request["janus"] = "trickle";
	std::string transaction_id = seeker::secure::randomChars(8);
	config_request["transaction"] = transaction_id;
	config_request["candidate"]["completed"] = true;
	auto res = cli->Post("/janus_restful/" + std::to_string(session_id) + "/" + std::to_string(handle_id), config_request.dump(), "application/json");
	if (res == nullptr) {
		E_LOG("janus handle link error");
		return false;
	}
	else {
		if (res->status == 200) {
			json response = json::parse(res->body);
			I_LOG("janus handle resp [{}]", response.dump());
			return true;
		}
		else {
			E_LOG("janus handle status error [{}]", res->status);
			return false;
		}
	}
}

void Conductor::getJanus(std::string transaction, bool sendsdp)
{
	auto res = cli->Get("/janus_restful/" + std::to_string(session_id));
	if (res == nullptr) {
		E_LOG("get error");
		return;
	}
	else {
		if (res->status == 200) {
			json response = json::parse(res->body);
			std::string retransaction = response["transaction"];
			if (retransaction == transaction) {
				I_LOG("janus get resp [{}]", response.dump());
			}
		}
	}
}

void Conductor::getJanu()
{
	auto res = cli->Get("/janus_restful/" + std::to_string(session_id));
	if (res == nullptr) {
		E_LOG("get error");
	}
	else {
		if (res->status == 200) {
			json response = json::parse(res->body);
			I_LOG("response body [{}]", response.dump());
			if (!response["jsep"]["sdp"].is_null()) {
				answersdp = response["jsep"]["sdp"];
				I_LOG("answersdp [{}]", answersdp);
				std::string type_str;
				type_str = response["jsep"]["type"];
				I_LOG("type_str [{}]", type_str);
				std::optional<webrtc::SdpType> type_maybe =
					webrtc::SdpTypeFromString(type_str);
				if (!type_maybe) {
					RTC_LOG(LS_ERROR) << "Unknown SDP type: " << type_str;
					return;
				}
				webrtc::SdpType type = *type_maybe;
				webrtc::SdpParseError error;
				std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
					webrtc::CreateSessionDescription(type, answersdp, &error);
				peer_connection_->SetRemoteDescription(
					DummySetSessionDescriptionObserver::Create().get(),
					session_description.release());
			}
		}
	}
}

void Conductor::keeplive()
{
	while (true) {
		json config_request;
		config_request["janus"] = "keepalive";
		std::string transaction_id = seeker::secure::randomChars(8);
		config_request["transaction"] = transaction_id;
		auto res = cli->Post("/janus_restful/" + std::to_string(session_id), config_request.dump(), "application/json");
		if (res == nullptr) {
			E_LOG("janus handle link error");
		}
		else {
			if (res->status == 200) {
				json response = json::parse(res->body);
				//I_LOG("janus handle resp [{}]", response.dump());
			}
			else {
				E_LOG("janus handle status error [{}]", res->status);
			}
		}
		Sleep(10000);
	}
}

//void Conductor::switchCamera(bool ifOpen) {
//	auto senders = peer_connection_->GetSenders();
//	for (auto& sender : senders) {
//		if (sender->track() && sender->track()->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
//			sender->track()->set_enabled(ifOpen);
//			//auto video_track = static_cast<rtc::scoped_refptr<webrtc::VideoTrackInterface>>(sender->track());
//			//video_track->set_enabled(ifOpen);
//		}
//	}
//}

void Conductor::replaceTrack(int index) {
	rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_ = nullptr;
	I_LOG("r 0.0");
	//ve->switchTrack(video_track_,index);
	ve->setCamera(index);
	
	I_LOG("r 1.0");
	//local_renderer_.release();
	//local_renderer_ = nullptr;
	//local_renderer_.reset(new VideoRenderer(std::bind(&Conductor::OnPaint, this), 1, 1, video_track_.get()));
	I_LOG("r 2.0");


	
	//ve->removeTrack();
	I_LOG("r 3.0");
	//switchCamera(false);
}

void Conductor::OnPaint() {
	//I_LOG("OnPaint");
	//获取本地和远端的视频画面
	VideoRenderer* local_renderer = local_renderer_.get();
	VideoRenderer* remote_renderer = remote_renderer_.get();
	VideoRenderer* screen_renderer = screen_renderer_.get();
	//if (!remote_renderer) I_LOG("remote_renderer nullptr");
	if (!local_renderer_) I_LOG("local_renderer_ nullptr");
	if (remote_renderer && local_renderer && screen_renderer_) {
	//if (remote_renderer && local_renderer) {
		AutoLock<VideoRenderer> local_lock(local_renderer);
		AutoLock<VideoRenderer> remote_lock(remote_renderer);
		AutoLock<VideoRenderer> screen_lock(screen_renderer);
		
		const BITMAPINFO& rbmi = remote_renderer->bmi();
		const uint8_t* rimage = remote_renderer->image();

		if (rimage != NULL) {

			remoteImageList.Push(ImageData(rbmi, rimage));
			//I_LOG("remoteImageList size:{}", remoteImageList.Size());

		}
		else {
			//E_LOG("rimage = NULL");
		}
		
		const BITMAPINFO& lbmi = local_renderer->bmi();
		const uint8_t* limage = local_renderer->image();

		if (limage != NULL) {

			localImageList.Push(ImageData(lbmi, limage));
			//I_LOG("localImageList size:{}", localImageList.Size());
			
		}
		else {
			E_LOG("111");
		}

		const BITMAPINFO& sbmi = screen_renderer->bmi();
		const uint8_t* simage = screen_renderer->image();

		if (simage != NULL) {
			screenImageList.Push(ImageData(sbmi, simage));
			//I_LOG("localImageList size:{}", localImageList.Size());

		}
		else {
			E_LOG("111");
		}

		

	}
	else {
		//E_LOG("222");
	}
}
