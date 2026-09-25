/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "conductor.h"
#include"utils/httplib.h"
#include "seeker/common.h"
//extern httplib::Client client;
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

int parseRes(const std::string url, const std::string& req_body, std::string& rsp_body) {
    httplib::Headers header;
    httplib::Client client{ "10.1.29.246", 8088 };
    I_LOG("send post requset url {} body {}", url, req_body);
    auto res = client.Post(url.c_str(), header, req_body, "application/json");
    if (res == nullptr) {
        E_LOG("Error");
        return -1;
    }
    else {
        if (res->status == 200) {
            json j;
            rsp_body = res->body;
            I_LOG("post request rsp {}", rsp_body);
        }
        else {
            E_LOG("Error");
            return -1;
        }
    }
    return 0;
}

int getHttpReq(const std::string url, std::string& rsp_body) {
    httplib::Headers header;
    httplib::Client client{ "10.1.29.246", 8088 };
    I_LOG("send get requset url {}", url);
    auto res = client.Get(url.c_str());
    if (res == nullptr) {
        E_LOG("Error");
        return -1;
    }
    else {
        if (res->status == 200) {
            json j;
            rsp_body = res->body;
            I_LOG("get request rsp {}", rsp_body);
        }
        else {
            E_LOG("Error");
            return -1;
        }
    }
    return 0;
}

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

    //rtc::scoped_refptr<CapturerTrackSource> video_device =
    //    CapturerTrackSource::Create();
    //if (video_device) {
    //    rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_(
    //        peer_connection_factory_->CreateVideoTrack(video_device, kVideoLabel));

    //    result_or_error = peer_connection_->AddTrack(video_track_, { kStreamId });
    //    if (!result_or_error.ok()) {
    //        RTC_LOG(LS_ERROR) << "Failed to add video track to PeerConnection: "
    //            << result_or_error.error().message();
    //    }
    //}
    //else {
    //    RTC_LOG(LS_ERROR) << "OpenVideoCaptureDevice failed";
    //}
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
        nullptr,
        nullptr,
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
    
}

Conductor::~Conductor() {

}

void Conductor::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
  I_LOG("Onsuccess");
  peer_connection_->SetLocalDescription(
      DummySetSessionDescriptionObserver::Create().get(), desc);
  desc->ToString(&sdp);
  sendMessageTojanusWithSDP(sdp);
}

void Conductor::OnIceGatheringChange(
    webrtc::PeerConnectionInterface::IceGatheringState new_state) {
    I_LOG("OnIceGatheringChange");
    if (new_state == webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete) {
        json j;
        std::string rsp_body;
        j["janus"] = "trickle";
        j["transaction"] = seeker::secure::randomChars(8);
        j["candidate"]["completed"] = true;
        parseRes(URL + "/" + std::to_string(sessionId) + "/" + std::to_string(handleId), j.dump(), rsp_body);
    }
    return;
}

void Conductor::OnFailure(webrtc::RTCError error) {
  
}


void  Conductor::OnAddTrack(
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
    const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
    streams) {}

void  Conductor::OnRemoveTrack(
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {

}

void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
{
    I_LOG("OnIceCandidate");
    Json::Value jmessage;
    jmessage[kCandidateSdpMidName] = candidate->sdp_mid();
    jmessage[kCandidateSdpMlineIndexName] = candidate->sdp_mline_index();
    if (!candidate->ToString(&sdp)) {
        E_LOG("error");
        return;
    }
    std::string rsp_body;
    json j;
    j["janus"] = "trickle";
    j["candidate"]["candidate"] = sdp;
    j["candidate"]["sdpMid"] = candidate->sdp_mid();
    j["candidate"]["sdpMLineIndex"] = candidate->sdp_mline_index();
    j["transaction"] = seeker::secure::randomChars(8);
    parseRes(URL + "/" + std::to_string(sessionId)+"/"+std::to_string(handleId), j.dump(), rsp_body);
    return;
}

void Conductor::createSession() {
    json j;
    j["janus"] = "create";
    j["transaction"] = seeker::secure::randomChars(8);
    std::string rsp_body;
    std::string url = URL;
   // I_LOG("httpCallCreate req_body {}", j.dump());
    int ret = parseRes(url, j.dump(), rsp_body);
    if (ret != 0) {
        return;
    }
    //I_LOG("httpCallCreate rsp_body {}", rsp_body);
    j = json::parse(rsp_body);
    if (j["janus"] != "success") {
        return;
    }
    sessionId = j["data"]["id"];
    //add handle
    j.clear();
    j["janus"] = "attach";
    j["plugin"] = "janus.plugin.echotest";
    j["transaction"] = seeker::secure::randomChars(8);

    url = URL + "/" + std::to_string(sessionId);
   // I_LOG("httpCallAttch req_body {}", j.dump());
    ret = parseRes(url, j.dump(), rsp_body);
    if (ret != 0) {
        return ;
    }
    j = json::parse(rsp_body);
    if (j["janus"] != "success") {
        return ;
    }
    handleId = j["data"]["id"];

    sendMessageToJanus();

    InitializePeerConnection();
    peer_connection_->CreateOffer(
        this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());

    std::thread working(&Conductor::keepAlive, this);
    working.detach();

    while (1) {
        Sleep(10000);
    }
    I_LOG("createSession end");
}

void Conductor::keepAlive() {
    httplib::Client client2{ "10.1.29.246", 8088 };
    while (true) {
        json j;
        j["janus"] = "keepalive";
        std::string transaction_id = seeker::secure::randomChars(8);
        j["transaction"] = transaction_id;
        std::string rsp_body;
        parseRes("/janus/" + std::to_string(sessionId), j.dump(), rsp_body);
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    return;
}

bool Conductor::sendMessageToJanus() {
    json j;
    j["janus"] = "message";
    j["transaction"] = seeker::secure::randomChars(8);
    j["body"]["audio"] = true;
    j["body"]["video"] = false;
    std::string rsp_body;
    parseRes(URL + "/"+std::to_string(sessionId) + "/" + std::to_string(handleId), j.dump(), rsp_body);
    
    getHttpReq("/janus/" + std::to_string(sessionId),rsp_body);
    j.clear();
    j = json::parse(rsp_body);
    std::string rspTransaction = j["transaction"];
    //if (rspTransaction != transaction) {
    //    E_LOG("janus error {}", j.dump());
    //}
    I_LOG("sendMessageToJanus end");
    return 0;
}

bool Conductor::sendMessageTojanusWithSDP(std::string SDP) {
    json j;
    j["janus"] = "message";
    j["transaction"] = seeker::secure::randomChars(8);
    j["body"]["audio"] = true;
    j["body"]["video"] = false;
    j["jsep"]["type"] = "offer";
    j["jsep"]["sdp"] = SDP;
    std::string rsp_body;
    parseRes(URL + "/" + std::to_string(sessionId) + "/" + std::to_string(handleId), j.dump(), rsp_body);

    getHttpReq("/janus/" + std::to_string(sessionId), rsp_body);
    j.clear();
    j = json::parse(rsp_body);
    //std::string rspTransaction = j["transaction"];
    //if (rspTransaction != transaction) {
    //    E_LOG("janus error {}", j.dump());
    //}
    std::string answerSdp = j["jsep"]["sdp"];
    std::string stringType = j["jsep"]["type"];
    std::optional<webrtc::SdpType> type_maybe =
        webrtc::SdpTypeFromString(stringType);
    if (!type_maybe) {
        E_LOG("error");
        return 0;
    }
    webrtc::SdpType type = *type_maybe;
    webrtc::SdpParseError error;
    std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
        webrtc::CreateSessionDescription(type, answerSdp, &error);
    peer_connection_->SetRemoteDescription(
        DummySetSessionDescriptionObserver::Create().get(),
        session_description.release());
    
    I_LOG("sendMessageTojanusWithSDP end");
    return 0;
}