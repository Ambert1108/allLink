#include "conductor.h"

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <optional>
#include <utility>
#include <vector>
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
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "p2p/base/port_allocator.h"
#include "pc/video_track_source.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "rtc_base/strings/json.h"
#include "test/vcm_capturer.h"
#include "defaults.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "media/engine/webrtc_media_engine.h"
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
            for (int i = 0; i < num_devices; ++i) {
                capturer = absl::WrapUnique(
                    webrtc::test::VcmCapturer::Create(kWidth, kHeight, kFps, i));
                if (capturer) {
                    return rtc::make_ref_counted<CapturerTrackSource>(std::move(capturer));
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
    class AudioRecorder : public webrtc::AudioTransport {
    public:
        AudioRecorder(){}

        virtual ~AudioRecorder() {
        }

        void StartRecording() {
            //recording = true;
        }
        void StopRecording() {
            //recording = false;
        }
        // 旧版本的RecordedDataIsAvailable
        int32_t RecordedDataIsAvailable(const void* audioSamples,
            size_t nSamples,
            size_t nBytesPerSample,
            size_t nChannels,
            uint32_t samplesPerSec,
            uint32_t totalDelayMS,
            int32_t clockDrift,
            uint32_t currentMicLevel,
            bool keyPressed,
            uint32_t& newMicLevel) override {
            return RecordedDataIsAvailable(audioSamples,
                nSamples,
                nBytesPerSample,
                nChannels,
                samplesPerSec,
                totalDelayMS,
                clockDrift,
                currentMicLevel,
                keyPressed,
                newMicLevel,
                std::nullopt);
        }
        int32_t RecordedDataIsAvailable(const void* audioSamples,
            size_t nSamples,
            size_t nBytesPerSample,
            size_t nChannels,
            uint32_t samplesPerSec,
            uint32_t totalDelayMS,
            int32_t clockDrift,
            uint32_t currentMicLevel,
            bool keyPressed,
            uint32_t& newMicLevel,
            std::optional<int64_t> estimatedCaptureTimeNS) override {
            int16_t buffer[nSamples * nChannels];
            CaptureSystemAudio(buffer, nSamples * nChannels);
            memcpy(const_cast<void*>(audioSamples), buffer, nSamples * nChannels * nBytesPerSample);
            I_LOG("1111");
            return 0;  // 返回0表示成功
        }

        int32_t NeedMorePlayData(size_t nSamples,
            size_t nBytesPerSample,
            size_t nChannels,
            uint32_t samplesPerSec,
            void* audioSamples,
            size_t& nSamplesOut,
            int64_t* elapsed_time_ms,
            int64_t* ntp_time_ms) override {
            nSamplesOut = 0;
            return 0;
        }

        void PullRenderData(int bits_per_sample,
            int sample_rate,
            size_t number_of_channels,
            size_t number_of_frames,
            void* audio_data,
            int64_t* elapsed_time_ms,
            int64_t* ntp_time_ms) override {
        }

    private:
        void CaptureSystemAudio(int16_t* buffer, int buffer_size) {
            // 这里实现从系统声音捕获音频数据的逻辑
            // 例如，使用WASAPI捕获音频数据并填充到buffer中

        }
    };
    
    //class CustomAudioEncoderFactory : public webrtc::AudioEncoderFactory {
    //public:
    //    std::vector<webrtc::AudioCodecSpec> GetSupportedEncoders() override {
    //        std::vector<webrtc::AudioCodecSpec> specs;
    //        webrtc::SdpAudioFormat pcma_format("PCMA", 8000, 1);
    //        webrtc::AudioCodecInfo pcma_info(8000, 1, 64000);  // 假设默认比特率为64000bps
    //        specs.emplace_back(pcma_format, pcma_info);
    //        return specs;
    //    }

    //    std::optional<webrtc::AudioCodecInfo> QueryAudioEncoder(
    //        const webrtc::SdpAudioFormat& format) override {
    //        if (format.name == "PCMA") {
    //            return webrtc::AudioCodecInfo(8000, 1, 64000);
    //        }
    //        return std::nullopt;
    //    }
    //    absl::Nullable<std::unique_ptr<webrtc::AudioEncoder>> Create(
    //        const webrtc::Environment& env,
    //        const webrtc::SdpAudioFormat& format,
    //        Options options) override {

    //    }
    //};
}
Conductor::Conductor(std::shared_ptr< httplib::Client> _cli){
    loopback_ = false;
    cli = _cli;
    rtcaudioEngine = std::make_shared<rtcengine::rtcAudioEngine>();
    //create();
    //attach();
    //deattach();
}
Conductor::~Conductor() {
	RTC_DCHECK(!peer_connection_);
}

bool Conductor::InitializePeerConnection()
{
    SetConsoleOutputCP(65001);
    RTC_DCHECK(!peer_connection_factory_);
    RTC_DCHECK(!peer_connection_);

    if (!signaling_thread_.get()) {
        signaling_thread_ = rtc::Thread::CreateWithSocketServer();
        signaling_thread_->Start();
    }
    adm = rtcaudioEngine->InitAdm();
    std::map<int16_t, std::string>Remap;
    std::map<int16_t, std::string>Plmap;

    rtcaudioEngine->GetRecordingDevices(Remap);
    for (auto& v : Remap) {
        I_LOG("key [{}] value [{}]", v.first, v.second);
    }

    rtcaudioEngine->GetPlayoutDevices(Plmap);
    for (auto& v : Plmap) {
        I_LOG("key [{}] value [{}]", v.first, v.second);
    }
    //rtcaudioEngine->ReplaceRecordingDevices(0);
    //rtcaudioEngine->ReplacePlayoutDevices(0);
    peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
        nullptr /* network_thread */, nullptr /* worker_thread */,
        signaling_thread_.get(), adm /* default_adm */,
        webrtc::CreateBuiltinAudioEncoderFactory(),
        webrtc::CreateBuiltinAudioDecoderFactory(),
        std::make_unique<webrtc::VideoEncoderFactoryTemplate<
        webrtc::LibvpxVp8EncoderTemplateAdapter,
        webrtc::LibvpxVp9EncoderTemplateAdapter,
        webrtc::OpenH264EncoderTemplateAdapter,
        webrtc::LibaomAv1EncoderTemplateAdapter>>(),
        std::make_unique<webrtc::VideoDecoderFactoryTemplate<
        webrtc::LibvpxVp8DecoderTemplateAdapter,
        webrtc::LibvpxVp9DecoderTemplateAdapter,
        webrtc::OpenH264DecoderTemplateAdapter,
        webrtc::Dav1dDecoderTemplateAdapter>>(),
        nullptr /* audio_mixer */, nullptr /* audio_processing */);
    
    if (!peer_connection_factory_) {
        DeletePeerConnection();
        return false;
    }

    if (!CreatePeerConnection()) {
        DeletePeerConnection();
    }
    //std::vector<webrtc::AudioCodecSpec> audio_specs;
    AddTracks();

    return peer_connection_ != nullptr;
}

bool Conductor::ReinitializePeerConnectionForLoopback()
{
    loopback_ = true;
    std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders =
        peer_connection_->GetSenders();
    peer_connection_ = nullptr;
    // Loopback is only possible if encryption is disabled.
    webrtc::PeerConnectionFactoryInterface::Options options;
    options.disable_encryption = true;
    peer_connection_factory_->SetOptions(options);
    if (CreatePeerConnection()) {
        for (const auto& sender : senders) {
            peer_connection_->AddTrack(sender->track(), sender->stream_ids());
        }
        peer_connection_->CreateOffer(
            this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    }
    options.disable_encryption = false;
    peer_connection_factory_->SetOptions(options);
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
    if (!peer_connection_->GetSenders().empty()) {
        return;  // Already added tracks.
    }
    rtcaudioEngine->AddAudioTracks(peer_connection_factory_, peer_connection_);

    //rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
    //    peer_connection_factory_->CreateAudioTrack(
    //        kAudioLabel,
    //        peer_connection_factory_->CreateAudioSource(cricket::AudioOptions())
    //        .get()));
    //at = audio_track;
    //auto result_or_error = peer_connection_->AddTrack(audio_track, { kStreamId });
    //I_LOG("id [{}]", audio_track.get()->id());
    //if (!result_or_error.ok()) {
    //    RTC_LOG(LS_ERROR) << "Failed to add audio track to PeerConnection: "
    //        << result_or_error.error().message();
    //}
    //rtc::scoped_refptr<CapturerTrackSource> video_device =
    //    CapturerTrackSource::Create();
    //rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_(
    //    peer_connection_factory_->CreateVideoTrack(video_device, kVideoLabel));
    //result_or_error = peer_connection_->AddTrack(video_track_, { kStreamId });
    //if (!result_or_error.ok()) {
    //    RTC_LOG(LS_ERROR) << "Failed to add video track to PeerConnection: "
    //        << result_or_error.error().message();
    //}
}

void Conductor::start()
{
    create();
    attach();
    sendmessage();
    if (InitializePeerConnection()) {
        peer_connection_->CreateOffer(
            this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    }
    else {
        E_LOG("InitializePeerConnection error");
    }

    std::thread lk(&Conductor::keeplive, this);
    lk.detach();
    while (true) {
        int x;
        std::cin >> x;
        if (x == 0) {
            rtcaudioEngine->ReplaceRecordingDevices(0);
            rtcaudioEngine->ReplacePlayoutDevices(0);
        }
        else if (x == 1) {
            rtcaudioEngine->ReplaceRecordingDevices(0);
            rtcaudioEngine->ReplacePlayoutDevices(1);
        }
        else if (x == 2) {
            rtcaudioEngine->ReplaceRecordingDevices(1);
            rtcaudioEngine->ReplacePlayoutDevices(0);
        }
        else if (x == 3) {
            rtcaudioEngine->ReplaceRecordingDevices(1);
            rtcaudioEngine->ReplacePlayoutDevices(1);
        }
        else if (x == 4) {
            rtcaudioEngine->setMicrophone(true);

        }
        else if (x == 5) {
            rtcaudioEngine->setMicrophone(false);

        }
        else if (x == 6) {
            int v;
            std::cin >> v;
            rtcaudioEngine->setPlayoutVolume(v);
        }
        else if (x == 7) {
            int v;
            std::cin >> v;
            rtcaudioEngine->setMicrophoneVolume(v);
            //adm->SetMicrophoneVolume(v);
        }
        else if (x == 8) {
            uint32_t v = rtcaudioEngine->MicrophoneVolume();
            //adm->MaxSpeakerVolume(&v);
            std::cout << v << std::endl;
        }
        else if (x == 9) {
            uint32_t v = rtcaudioEngine->PlayoutVolume();
            //adm->MaxSpeakerVolume(&v);
            std::cout << v << std::endl;
        }
    }
    while (true) {
        Sleep(10000);
    }
}

void Conductor::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams)
{
    RTC_LOG(LS_INFO) << __FUNCTION__ << " " << receiver->id();
}

void Conductor::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)
{
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
    //peer_connection_->SetLocalDescription(
    //    DummySetSessionDescriptionObserver::Create().get(), desc);

    std::string sdp;
    desc->ToString(&sdp);
    I_LOG("SDP [{}]", sdp);
    std::string sdptemp = rtcaudioEngine->modifySdp(sdp);
    I_LOG("sdptemp [{}]", sdptemp);

    std::string type_str="offer";
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
        webrtc::CreateSessionDescription(type, sdptemp, &error);
    peer_connection_->SetLocalDescription(
        DummySetSessionDescriptionObserver::Create().get(),
        session_description.release());
    // For loopback test. To save some connecting delay.
    //if (loopback_) {
    //    // Replace message type from "offer" to "answer"
    //    std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
    //        webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, sdp);
    //    peer_connection_->SetRemoteDescription(
    //        DummySetSessionDescriptionObserver::Create().get(),
    //        session_description.release());
    //    return;
    //}
    sendoffersdp(sdptemp);
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
    auto res = cli->Post("/janus", create_session_request.dump(), "application/json");
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
    // 附加到EchoTest插件
    json create_plug_request;
    create_plug_request["janus"] = "attach";
    std::string transaction_id = seeker::secure::randomChars(8);
    create_plug_request["transaction"] = transaction_id;
    create_plug_request["plugin"] = "janus.plugin.echotest";
    auto res = cli->Post("/janus/" + std::to_string(session_id), create_plug_request.dump(), "application/json");
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
    config_request["body"]["audio"] = true;
    config_request["body"]["video"] = false;
    auto res = cli->Post("/janus/" + std::to_string(session_id) + "/" + std::to_string(handle_id), config_request.dump(), "application/json");
    if (res == nullptr) {
        E_LOG("janus handle link error");
        return false;
    }
    else {
        if (res->status == 200) {
            json response = json::parse(res->body);
            I_LOG("janus handle resp [{}]", response.dump());
            getJanus(transaction_id, false);
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
    config_request["body"]["audio"] = true;
    config_request["body"]["video"] = false;
    config_request["jsep"]["type"] = "offer";
    config_request["jsep"]["sdp"] = offersdp;
    auto res = cli->Post("/janus/" + std::to_string(session_id) + "/" + std::to_string(handle_id), config_request.dump(), "application/json");
    if (res == nullptr) {
        E_LOG("janus handle link error");
        return false;
    }
    else {
        if (res->status == 200) {
            json response = json::parse(res->body);
            I_LOG("janus handle resp [{}]", response.dump());
            getJanus(transaction_id, true);
            //offid = transaction_id;
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
    auto res = cli->Post("/janus/" + std::to_string(session_id) + "/" + std::to_string(handle_id), config_request.dump(), "application/json");
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
    auto res = cli->Post("/janus/" + std::to_string(session_id) + "/" + std::to_string(handle_id), config_request.dump(), "application/json");
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
	auto res = cli->Get("/janus/" + std::to_string(session_id));
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
                if (sendsdp) {
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
	}
}

void Conductor::keeplive()
{
    while (true) {
        json config_request;
        config_request["janus"] = "keepalive";
        std::string transaction_id = seeker::secure::randomChars(8);
        config_request["transaction"] = transaction_id;
        auto res = cli->Post("/janus/" + std::to_string(session_id), config_request.dump(), "application/json");
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













