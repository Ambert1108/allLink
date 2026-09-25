#include "conductor.h"
//#include"utils/httplib.h"
#include "seeker/common.h"

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
      : VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {}

 private:
  rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
    return capturer_.get();
  }
  std::unique_ptr<webrtc::test::VcmCapturer> capturer_;
};

}  // namespace

void Conductor::AddTracks() {
    I_LOG("AddTracks begin");
    if (!peer_connection_->GetSenders().empty()) {
        return;  // Already added tracks.
    }

    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
        peer_connection_factory_->CreateAudioTrack(
            kAudioLabel,
            peer_connection_factory_->CreateAudioSource(cricket::AudioOptions())
            .get()));
    auto result_or_error = peer_connection_->AddTrack(audio_track, { kStreamId });
    if (!result_or_error.ok()) {
        RTC_LOG(LS_ERROR) << "Failed to add audio track to PeerConnection: "
            << result_or_error.error().message();
    }

    rtc::scoped_refptr<CapturerTrackSource> video_device =
        CapturerTrackSource::Create();
    if (video_device) {
        rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_(
            peer_connection_factory_->CreateVideoTrack(video_device, kVideoLabel));
    
        result_or_error = peer_connection_->AddTrack(video_track_, { kStreamId });
        if (!result_or_error.ok()) {
            RTC_LOG(LS_ERROR) << "Failed to add video track to PeerConnection: "
                << result_or_error.error().message();
        }
    }
    else {
        RTC_LOG(LS_ERROR) << "OpenVideoCaptureDevice failed";
    }


    I_LOG("AddTracks end");
}

bool Conductor::InitializePeerConnection()
{
    I_LOG("InitializePeerConnection begin");
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
        //nullptr,
        //nullptr,
        nullptr /* audio_mixer */, nullptr /* audio_processing */);


    if (!peer_connection_factory_) {
        E_LOG("error");
        //DeletePeerConnection();
        return false;
    }

    if (!CreatePeerConnection()) {
        E_LOG("error");
        return false;
        //DeletePeerConnection();
    }

    AddTracks();
    I_LOG("InitializePeerConnection end");
    return peer_connection_ != nullptr;
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
    auto error_or_peer_connection =
        peer_connection_factory_->CreatePeerConnectionOrError(
            config, std::move(pc_dependencies));
    if (error_or_peer_connection.ok()) {
        peer_connection_ = std::move(error_or_peer_connection.value());
    }
    return peer_connection_ != nullptr;
}


Conductor::Conductor(){
    client = std::make_shared<Janitor>("10.1.29.246", 8188);
    client->registerObserver(this);
    client->init();
    I_LOG("client init");
}

Conductor::~Conductor() {

}

void Conductor::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
  I_LOG("Onsuccess is callback");
  peer_connection_->SetLocalDescription(
      DummySetSessionDescriptionObserver::Create().get(), desc);
  desc->ToString(&sdp);
  client->generateSDP(sdp,"offer");
  //sendMessageTojanusWithSDP(sdp);
}

void Conductor::OnIceGatheringChange(
    webrtc::PeerConnectionInterface::IceGatheringState new_state) {
    I_LOG("OnIceGatheringChange");
    if (new_state == webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete) {
 //// send completed
        //json j;
        //std::string rsp_body;
        //j["janus"] = "trickle";
        //j["session_id"] = sessionId;
        //j["handle_id"] = handleId;
        //j["transaction"] = seeker::secure::randomChars(8);
        //j["candidate"]["completed"] = true;
        ////parseRes(URL + "/" + std::to_string(sessionId) + "/" + std::to_string(handleId), j.dump(), rsp_body);
        //client->sendMessageToJanus(j.dump());
        client->sendTrickleCompleteToJanus();
    }
    return;
}

void Conductor::OnFailure(webrtc::RTCError error) {
    I_LOG("OnFailure is callback");
}


void  Conductor::OnAddTrack(
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
    const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
    streams) {
    I_LOG("OnAddTrack is callback");
}

void  Conductor::OnRemoveTrack(
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {

}

void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
{
    I_LOG("OnIceCandidate");
    //Json::Value jmessage;
    //jmessage[kCandidateSdpMidName] = candidate->sdp_mid();
    //jmessage[kCandidateSdpMlineIndexName] = candidate->sdp_mline_index();
    if (!candidate->ToString(&sdp)) {
        E_LOG("error");
        return;
    }
    json j;
    j["sdpMid"] = candidate->sdp_mid();
    j["sdpMLineIndex"] = candidate->sdp_mline_index();
    j["candidate"] = sdp;
   
    
    client->sendTrickleToJanus(j.dump());
    return;
}

void Conductor::createSession() {
    I_LOG("createSession");
    Sleep(1000);

    InitializePeerConnection();
    peer_connection_->CreateOffer(
        this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());

    //Sleep(30000);
    //while (1) {
        Sleep(10000);
    //}
    //Sleep(20000);
    I_LOG("createSession end");
}

void Conductor::resetPlugin() {


    //peer_connection_ = nullptr;
    //peer_connection_factory_ = nullptr;

    I_LOG("resetPlugin");
    Sleep(1000);

    client->addNoSIP();

    Sleep(1000);

    createSession();
}



bool Conductor::sendMessageTojanusWithSDP(std::string SDP) {
    I_LOG("sendMessageTojanusWithSDP");
    //json j;
    //j["janus"] = "message";
    //j["transaction"] = seeker::secure::randomChars(8);
    //j["body"]["audio"] = true;
    //j["body"]["video"] = false;
    //j["jsep"]["type"] = "offer";
    //j["jsep"]["sdp"] = SDP;
    //std::string rsp_body;
    //parseRes(URL + "/" + std::to_string(sessionId) + "/" + std::to_string(handleId), j.dump(), rsp_body);
    //client->generateSDP(SDP);
    //
    //getHttpReq("/janus/" + std::to_string(sessionId), rsp_body);
    //j.clear();
    //j = json::parse(rsp_body);
    //std::string rspTransaction = j["transaction"];
    //if (rspTransaction != transaction) {
    //    E_LOG("janus error {}", j.dump());
    ////}
    //std::string answerSdp = j["jsep"]["sdp"];
    //std::string stringType = j["jsep"]["type"];
    //std::optional<webrtc::SdpType> type_maybe =
    //    webrtc::SdpTypeFromString(stringType);
    //if (!type_maybe) {
    //    E_LOG("error");
    //    return 0;
    //}
    //
    //std::string audioIp;
    //int audioPort;
    //getAudioAddressBySdp(SDP, audioIp, audioPort);

    //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //std::string answerSdp = "v=0\r\no=- 4262987656738261604 2 IN IP4 1.1.1.1\r\ns=-\r\nt=0 0\r\nm=audio 61000 RTP/AVP 111 63 9 102 0 8 13 110 126\r\nc=IN IP4 10.4.6.151\r\na=sendrecv\r\na=mid:0\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=msid:stream_id audio_label\r\na=rtpmap:8 PCMA/8000\r\nm=video 61002 RTP/AVP 96 98 100 39\r\nc=IN IP4 10.4.6.151\r\na=sendrecv\r\na=mid:1\r\na=extmap:14 urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:13 urn:3gpp:video-orientation\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay\r\na=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type\r\na=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing\r\na=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\r\na=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\r\na=msid:stream_id video_label\r\na=rtpmap:96 VP8/90000\r\na=rtcp-fb:96 goog-remb\r\na=rtcp-fb:96 transport-cc\r\na=rtcp-fb:96 ccm fir\r\na=rtcp-fb:96 nack\r\na=rtcp-fb:96 nack pli\r\na=rtpmap:98 VP9/90000\r\na=rtcp-fb:98 goog-remb\r\na=rtcp-fb:98 transport-cc\r\na=rtcp-fb:98 ccm fir\r\na=rtcp-fb:98 nack\r\na=rtcp-fb:98 nack pli\r\na=fmtp:98 profile-id=0\r\na=rtpmap:100 VP9/90000\r\na=rtcp-fb:100 goog-remb\r\na=rtcp-fb:100 transport-cc\r\na=rtcp-fb:100 ccm fir\r\na=rtcp-fb:100 nack\r\na=rtcp-fb:100 nack pli\r\na=fmtp:100 profile-id=2\r\na=rtpmap:39 AV1/90000\r\na=rtcp-fb:39 goog-remb\r\na=rtcp-fb:39 transport-cc\r\na=rtcp-fb:39 ccm fir\r\na=rtcp-fb:39 nack\r\na=rtcp-fb:39 nack pli\r\na=fmtp:39 level-idx=5;profile=0;tier=0\r\n";
    //
    //std::string normalSdp = "v=0\r\no=- 5816815383681298367 2 IN IP4 1.1.1.1\r\ns=-\r\nt=0 0\r\nm=audio 62222 RTP/AVP 111 63 9 102 0 8 13 110 126\r\nc=IN IP4 10.4.7.64\r\na=sendrecv\r\na=mid:0\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=msid:stream_id audio_label\r\na=rtpmap:111 opus/48000/2\r\na=rtcp-fb:111 transport-cc\r\na=fmtp:111 minptime=10;useinbandfec=1\r\na=rtpmap:63 red/48000/2\r\na=fmtp:63 111/111\r\na=rtpmap:9 G722/8000\r\na=rtpmap:102 ILBC/8000\r\na=rtpmap:0 PCMU/8000\r\na=rtpmap:8 PCMA/8000\r\na=rtpmap:13 CN/8000\r\na=rtpmap:110 telephone-event/48000\r\na=rtpmap:126 telephone-event/8000\r\nm=video 21346 RTP/AVP 96 98 100 39\r\nc=IN IP4 10.1.29.246\r\na=sendrecv\r\na=mid:1\r\na=extmap:14 urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:13 urn:3gpp:video-orientation\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay\r\na=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type\r\na=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing\r\na=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\r\na=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\r\na=msid:stream_id video_label\r\na=rtpmap:96 VP8/90000\r\na=rtcp-fb:96 goog-remb\r\na=rtcp-fb:96 transport-cc\r\na=rtcp-fb:96 ccm fir\r\na=rtcp-fb:96 nack\r\na=rtcp-fb:96 nack pli\r\na=rtpmap:98 VP9/90000\r\na=rtcp-fb:98 goog-remb\r\na=rtcp-fb:98 transport-cc\r\na=rtcp-fb:98 ccm fir\r\na=rtcp-fb:98 nack\r\na=rtcp-fb:98 nack pli\r\na=fmtp:98 profile-id=0\r\na=rtpmap:100 VP9/90000\r\na=rtcp-fb:100 goog-remb\r\na=rtcp-fb:100 transport-cc\r\na=rtcp-fb:100 ccm fir\r\na=rtcp-fb:100 nack\r\na=rtcp-fb:100 nack pli\r\na=fmtp:100 profile-id=2\r\na=rtpmap:39 AV1/90000\r\na=rtcp-fb:39 goog-remb\r\na=rtcp-fb:39 transport-cc\r\na=rtcp-fb:39 ccm fir\r\na=rtcp-fb:39 nack\r\na=rtcp-fb:39 nack pli\r\na=fmtp:39 level-idx=5;profile=0;tier=0\r\n\r\n";
    //client->processSDP(normalSdp);

    //std::string answerSdp = observer.jsepSDP;
    //std::string stringType = "answer";
    //std::optional<webrtc::SdpType> type_maybe = webrtc::SdpTypeFromString(stringType);
    //if (!type_maybe) {
    //    E_LOG("error");
    //    return 0;
    //}
    //
    //webrtc::SdpType type = *type_maybe;
    //webrtc::SdpParseError error;
    //std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
    //    webrtc::CreateSessionDescription(type, answerSdp, &error);
    std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
        webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, SDP);
    peer_connection_->SetRemoteDescription(
        DummySetSessionDescriptionObserver::Create().get(),
        session_description.release());
    
    I_LOG("sendMessageTojanusWithSDP end");
    return 0;
}


void Conductor::OnReconnect() {

    client->init();
    I_LOG("client init");
    createSession();
    //std::thread loop1{ &Conductor::createSession,this};
    //loop1.detach();
}

void Conductor::OnGenerated(const std::string& sdp, const std::string& type) {
    I_LOG("OnGenerated");
    //std::string normalsdp = "v=0\r\no=- 6382569794913412367 2 IN IP4 1.1.1.1\r\ns=-\r\nt=0 0\r\nm=audio 62222 RTP/AVP 111 63 9 102 0 8 13 110 126\r\nc=IN IP4 10.4.7.64\r\na=sendrecv\r\na=mid:0\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=msid:stream_id audio_label\r\na=rtpmap:111 opus/48000/2\r\na=rtcp-fb:111 transport-cc\r\na=fmtp:111 minptime=10;useinbandfec=1\r\na=rtpmap:63 red/48000/2\r\na=fmtp:63 111/111\r\na=rtpmap:9 G722/8000\r\na=rtpmap:102 ILBC/8000\r\na=rtpmap:0 PCMU/8000\r\na=rtpmap:8 PCMA/8000\r\na=rtpmap:13 CN/8000\r\na=rtpmap:110 telephone-event/48000\r\na=rtpmap:126 telephone-event/8000\r\nm=video 63333 RTP/AVP 96 98 100 39\r\nc=IN IP4 10.4.7.64\r\na=sendrecv\r\na=mid:1\r\na=extmap:14 urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:13 urn:3gpp:video-orientation\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay\r\na=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type\r\na=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing\r\na=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\r\na=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\r\na=msid:stream_id video_label\r\na=rtpmap:96 VP8/90000\r\na=rtcp-fb:96 goog-remb\r\na=rtcp-fb:96 transport-cc\r\na=rtcp-fb:96 ccm fir\r\na=rtcp-fb:96 nack\r\na=rtcp-fb:96 nack pli\r\na=rtpmap:98 VP9/90000\r\na=rtcp-fb:98 goog-remb\r\na=rtcp-fb:98 transport-cc\r\na=rtcp-fb:98 ccm fir\r\na=rtcp-fb:98 nack\r\na=rtcp-fb:98 nack pli\r\na=fmtp:98 profile-id=0\r\na=rtpmap:100 VP9/90000\r\na=rtcp-fb:100 goog-remb\r\na=rtcp-fb:100 transport-cc\r\na=rtcp-fb:100 ccm fir\r\na=rtcp-fb:100 nack\r\na=rtcp-fb:100 nack pli\r\na=fmtp:100 profile-id=2\r\na=rtpmap:39 AV1/90000\r\na=rtcp-fb:39 goog-remb\r\na=rtcp-fb:39 transport-cc\r\na=rtcp-fb:39 ccm fir\r\na=rtcp-fb:39 nack\r\na=rtcp-fb:39 nack pli\r\na=fmtp:39 level-idx=5;profile=0;tier=0\r\n";
    std::string normalsdp = "v=0\r\no=- 6382569794913412367 2 IN IP4 1.1.1.1\r\ns=-\r\nt=0 0\r\nm=audio 62222 RTP/AVP 9 8\r\nc=IN IP4 10.4.7.64\r\na=sendrecv\r\na=mid:0\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=msid:stream_id audio_label\r\na=rtpmap:9 G722/8000\r\na=rtpmap:8 PCMA/8000\r\nm=video 63333 RTP/AVP 96 98\r\nc=IN IP4 10.4.7.64\r\na=sendrecv\r\na=mid:1\r\na=extmap:14 urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:13 urn:3gpp:video-orientation\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay\r\na=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type\r\na=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing\r\na=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\r\na=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\r\na=msid:stream_id video_label\r\na=rtpmap:96 VP8/90000\r\na=rtcp-fb:96 goog-remb\r\na=rtcp-fb:96 transport-cc\r\na=rtcp-fb:96 ccm fir\r\na=rtcp-fb:96 nack\r\na=rtcp-fb:96 nack pli\r\na=rtpmap:98 VP9/90000\r\na=rtcp-fb:98 goog-remb\r\na=rtcp-fb:98 transport-cc\r\na=rtcp-fb:98 ccm fir\r\na=rtcp-fb:98 nack\r\na=rtcp-fb:98 nack pli\r\na=fmtp:98 profile-id=0\r\n";
    client->processSDP(normalsdp,"answer");
}

void Conductor::OnProcessed(const std::string& sdp, const std::string& type) {
    I_LOG("OnProcessed");
    sendMessageTojanusWithSDP(sdp);
}

inline int Conductor::changeSdp(std::string& sdpString, const std::string accessPointIp, const int accessPointPort, const int mediaType)
{
    const static std::string ipRegString2 = R"regex(IN IP4 (\d+).(\d+).(\d+).(\d+))regex";
    const static std::regex ipReg2(ipRegString2);
    const static std::string portRegString = R"regex(m=video (\d+) RTP)regex";
    const static std::regex portReg(portRegString);
    const static std::string audioPortRegString = R"regex(m=audio (\d+) RTP)regex";
    const static std::regex audioPortReg(audioPortRegString);

    std::smatch sm;

    std::string::const_iterator videoSesionBegin = sdpString.begin();
    std::string::const_iterator videoSesionEnd = sdpString.end();

    int flag = 0;
    if (mediaType == 1 || mediaType == 2) {//�ı�Ip��ַ
        while (1) {
            if (std::regex_search(videoSesionBegin, videoSesionEnd, sm, ipReg2)) {
                if (std::string(sm[1].first, sm[4].second) == accessPointIp) {
                    videoSesionBegin = sm[4].second;
                }
                else {
                    sdpString.replace(sm[1].first, sm[4].second, accessPointIp);
                    videoSesionBegin = sdpString.begin();
                    videoSesionEnd = sdpString.end();
                    flag = 1;
                }
            }
            else {
                break;
            }
        }
        if (flag == 0) {
            E_LOG("fail to change ip");
            return -1;
        }
    }

    if (mediaType == 1 || mediaType == 2) {//��Ƶ/����Ƶ
        if (std::regex_search(sdpString, sm, portReg)) {//����Ƶport
            sdpString.replace(sm[1].first, sm[1].second, std::to_string(accessPointPort));
        }
        else {
            E_LOG("fail to change port");
        }

        if (mediaType == 1) {//����Ƶ������Ƶ�˿���0
            if (std::regex_search(sdpString, sm, audioPortReg)) {//����Ƶport
                sdpString.replace(sm[1].first, sm[1].second, "0");
            }
            else {
                E_LOG("fail to change audio port");
            }
        }
    }

    if (mediaType == 0) {//����Ƶ�£�����Ƶ�˿���0
        if (std::regex_search(sdpString, sm, portReg)) {//����Ƶport
            sdpString.replace(sm[1].first, sm[1].second, "0");
        }
        else {
            E_LOG("fail to change video port");
        }
    }
    return 0;
}