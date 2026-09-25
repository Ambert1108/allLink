//
// Created by 姚惠晶 on 2024/11/29.
//

#include "conductor.h"

// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";

class DummySetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver {
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
            capturer = absl::WrapUnique(webrtc::test::VcmCapturer::Create(kWidth, kHeight, kFps, i));
            if (capturer) {
                return rtc::make_ref_counted<CapturerTrackSource>(std::move(capturer));
            }
        }

        return nullptr;
    }

protected:
    explicit CapturerTrackSource(
            std::unique_ptr<webrtc::test::VcmCapturer> capturer)
            : VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {}

private:
    rtc::VideoSourceInterface<webrtc::VideoFrame> *source() override {
        return capturer_.get();
    }

    std::unique_ptr<webrtc::test::VcmCapturer> capturer_;
};

Conductor::Conductor(std::shared_ptr<ConnectEngine> connect_) {
    this->connect = connect_;
}

Conductor::~Conductor() {
    RTC_DCHECK(!peer_connection_);
}

bool Conductor::InitializePeerConnection() {
    RTC_DCHECK(!peer_connection_factory_);
    RTC_DCHECK(!peer_connection_);

    if (!signaling_thread_.get()) {
        signaling_thread_ = rtc::Thread::CreateWithSocketServer();
        signaling_thread_->Start();
    }
    peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
            nullptr /* network_thread */, nullptr /* worker_thread */,
            signaling_thread_.get(), nullptr /* default_adm */,
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

    AddTracks();

    peer_connection_->CreateOffer(this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());

    return peer_connection_ != nullptr;
}

void Conductor::AddTracks() {
    if (!peer_connection_->GetSenders().empty()) {
        return;  // Already added tracks.
    }

    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(peer_connection_factory_->CreateAudioTrack(kAudioLabel,
                                                                                                           peer_connection_factory_->CreateAudioSource(
                                                                                                                   cricket::AudioOptions()).get()));
    auto result_or_error = peer_connection_->AddTrack(audio_track, {kStreamId});
    if (!result_or_error.ok()) {
        RTC_LOG(LS_ERROR) << "Failed to add audio track to PeerConnection: " << result_or_error.error().message();
    }
    rtc::scoped_refptr<CapturerTrackSource> video_device = CapturerTrackSource::Create();
    if (video_device) {
        rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_(peer_connection_factory_->CreateVideoTrack(video_device, kVideoLabel));
        result_or_error = peer_connection_->AddTrack(video_track_, {kStreamId});
        if (!result_or_error.ok()) {
            RTC_LOG(LS_ERROR) << "Failed to add video track to PeerConnection: " << result_or_error.error().message();
        }
    } else {
        RTC_LOG(LS_ERROR) << "OpenVideoCaptureDevice failed";
    }

}

bool Conductor::CreatePeerConnection() {
    RTC_DCHECK(peer_connection_factory_);
    RTC_DCHECK(!peer_connection_);

    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = GetPeerConnectionString();
    config.servers.push_back(server);

    webrtc::PeerConnectionDependencies pc_dependencies(this);
    auto error_or_peer_connection = peer_connection_factory_->CreatePeerConnectionOrError(config,
                                                                                          std::move(pc_dependencies));
    if (error_or_peer_connection.ok()) {
        peer_connection_ = std::move(error_or_peer_connection.value());
    }
    return peer_connection_ != nullptr;
}

void Conductor::DeletePeerConnection() {
    peer_connection_ = nullptr;
    peer_connection_factory_ = nullptr;
    peer_id_ = -1;
    loopback_ = false;
}


void Conductor::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
                           const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>> &streams) {
    I_LOG("OnAddTrack");
    RTC_LOG(LS_INFO) << __FUNCTION__ << " " << receiver->id();
}

void Conductor::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
    I_LOG("OnRemoveTrack");
    RTC_LOG(LS_INFO) << __FUNCTION__ << " " << receiver->id();
}

void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface *candidate) {
    I_LOG("OnIceCandidate");
    RTC_LOG(LS_INFO) << __FUNCTION__ << " " << candidate->sdp_mline_index();
    peer_connection_->AddIceCandidate(candidate);

    connect->sendTrickle(candidate);
}

void Conductor::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
    I_LOG("OnIceGatheringChange");
    sendCandidateDone = true;
    if(new_state == webrtc::PeerConnectionInterface::kIceGatheringComplete) {
        connect->sendTrickleComplete();
    }
}

void Conductor::OnSuccess(webrtc::SessionDescriptionInterface *desc) {
    I_LOG("onSuccess");
    peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create().get(), desc);

    std::string sdp;
    desc->ToString(&sdp);
//    I_LOG("jsep : {}", sdp1);
//
//    std::string sdp = filterSDP(sdp1, true, false);
//    I_LOG("filterJsep: {}", sdp);

//    std::unique_ptr<webrtc::SessionDescriptionInterface> session_description = webrtc::CreateSessionDescription(webrtc::SdpType::kOffer, sdp);
//    peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create().get(),session_description.release());


    jsepQueue.push(sdp);

    // For loopback test. To save some connecting delay.
    if (loopback_) {
        // Replace message type from "offer" to "answer"
        std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
                webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, sdp);
        peer_connection_->SetRemoteDescription(
                DummySetSessionDescriptionObserver::Create().get(),
                session_description.release());
        return;
    }
    Json::StreamWriterBuilder factory;
}

void Conductor::OnFailure(webrtc::RTCError error) {
    I_LOG("OnFailure");
    RTC_LOG(LS_ERROR) << ToString(error.type()) << ": " << error.message();
}

std::string Conductor::getJsep() {
    std::string tmp = "";
    if (jsepQueue.size()) {
        tmp = jsepQueue.front();
        jsepQueue.pop();
    }
    return tmp;
}

void Conductor::setRemote(std::string jsep) {
    std::unique_ptr<webrtc::SessionDescriptionInterface> session_description = webrtc::CreateSessionDescription(
            webrtc::SdpType::kAnswer, jsep);
    peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create().get(),
                                           session_description.release());
}


//bool Conductor::shouldKeepCodec(const std::string& line) {
//    // 只保留PCMA和H264协议相关的行
//    return (line.find("PCMA") != std::string::npos ||
//            line.find("H264") != std::string::npos);
//}
//
//std::string Conductor::filterSDP(const std::string& sdp, bool audioflag,bool videoflag) {
//    std::istringstream sdpStream(sdp);
//    std::ostringstream filteredSDP;
//    std::string line;
//    bool inAudio = false;
//    bool inVideo = false;
//    bool inrtpmat = false;
//    bool inpcma = false;
//    bool inH264 = false;
//    bool vf = false;
//    bool af = false;
//    // 逐行读取原始SDP内容
//    while (std::getline(sdpStream, line)) {
//        if (line.find("m=audio") != std::string::npos) {
//            inVideo = false;
//            inAudio = true;  // 进入音频部分
//            inH264 = false;
//            inpcma = false;
//            inrtpmat = false;
//            vf = false;
//            af = false;
//            filteredSDP << line << std::endl;
//        }
//        else if (line.find("m=video") != std::string::npos) {
//            inAudio = false; // 离开音频部分
//            inVideo = true;  // 进入视频部分
//            inH264 = false;
//            inpcma = false;
//            inrtpmat = false;
//            vf = false;
//            af = false;
//            filteredSDP << line << std::endl;
//        }
//        else if (inAudio) {
//            if (audioflag) {
//                if (line.find("a=rtpmap") != std::string::npos) {
//                    inrtpmat = true;
//                    af = true;
//                }
//                else {
//                    inrtpmat = false;
//                }
//                if (inrtpmat) {
//                    if (shouldKeepCodec(line)) {
//                        inpcma = true;
//                        filteredSDP << line << std::endl;
//                    }
//                    else {
//                        inpcma = false;
//                    }
//                }
//                else {
//                    if (inpcma) {
//                        filteredSDP << line << std::endl;
//                    }
//                    else if (!af) {
//                        filteredSDP << line << std::endl;
//                    }
//                }
//            }
//            else {
//                filteredSDP << line << std::endl;
//            }
//        }
//        else if (inVideo) {
//            if (videoflag) {
//                if (line.find("a=rtpmap") != std::string::npos) {
//                    inrtpmat = true;
//                    vf = true;
//                }
//                else {
//                    inrtpmat = false;
//                }
//                if (inrtpmat) {
//                    if (shouldKeepCodec(line)) {
//                        inH264 = true;
//                        filteredSDP << line << std::endl;
//                    }
//                    else {
//                        inH264 = false;
//                    }
//                }
//                else {
//                    if (inH264) {
//                        filteredSDP << line << std::endl;
//                    }
//                    else if (!vf) {
//                        filteredSDP << line << std::endl;
//                    }
//                }
//            }
//            else {
//                filteredSDP << line << std::endl;
//            }
//        }
//        else {
//            filteredSDP << line << std::endl;
//        }
//    }
//
//    return filteredSDP.str();
//}