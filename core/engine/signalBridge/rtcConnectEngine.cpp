//
// Created by 姚惠晶 on 2025/1/7.
//

#include "rtcConnectEngine.h"

namespace rtcengine {
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

    RtcConnectEngine::RtcConnectEngine() {}

    RtcConnectEngine::~RtcConnectEngine() {
        threadDestroy = true;
        if (keepaliveThread.joinable()) {
            keepaliveThread.join();
        }
        D_LOG("keepaliveThread.join");

        if(getDevInfoThread.joinable()){
            getDevInfoThread.join();
            D_LOG("getDevInfoThread.join");
        }

        std::unique_lock<std::mutex> lock(stopMtx);
        stopCv.wait(lock, [this]{ return meetingId.empty(); });
        D_LOG("stopCv notify");

        signalingSocket->sendClose();
        D_LOG("sendClose");

        if (listenerThread.joinable()) {
            listenerThread.join();
        }
        D_LOG("listenerThread.join");
    }

    bool RtcConnectEngine::connect(std::string signalIp_, uint16_t signalPort_) {
        try {
            auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({signalIp_, signalPort_});
            D_LOG("signaling addr: {}:{}", signalIp_, signalPort_);

            auto connector = oatpp::websocket::Connector::createShared(connectionProvider);
            auto connection = connector->connect("/connectWS");
            signalInfo.signalIp = signalIp_;
            signalInfo.signalPort = signalPort_;
            D_LOG("signaling ws connected done!");

            signalingSocket = oatpp::websocket::WebSocket::createShared(connection,
                                                                        true /* maskOutgoingMessages must be true for clients */);
            std::mutex socketWriteMutex;
            signalingListener = std::make_shared<EngineListener>(socketWriteMutex);
            signalingSocket->setListener(signalingListener);
            signalingListener->RegisterObserver(this);

            std::thread threadListener(&RtcConnectEngine::socketTask, this, signalingSocket);
            listenerThread = std::move(threadListener);

            if(!keepaliveThread.joinable()){
                std::thread threadKeepalive(&RtcConnectEngine::keepalive, this);
                keepaliveThread = std::move(threadKeepalive);
            }
            signalState = State::CONNECT_ON;
        }
        catch (...) {
            return false;
        }
        return true;
    }

    bool RtcConnectEngine::login(std::string userId_, std::string password_) {
        if (!userId_.empty() && !password_.empty()) {
            Message loginReq;
            loginReq.set_userid(userId_);
            loginReq.set_password(password_);
            loginReq.set_cseq(cseq++);
            loginReq.set_meth("REGISTER");
            loginReq.set_isresponse(false);
            D_LOG("login Req: {}", loginReq.js.dump(4));
            oatpp::String loginJson = oatpp::String(loginReq.js.dump());
            sendSocket(loginJson);

            userInfo.userId = userId_;
            userInfo.password = password_;
        } else {
            E_LOG("userId/password is empty");
            return false;
        }
        return true;
    }

    bool RtcConnectEngine::joinMeeting(std::string meetingId_) {
        std::regex pattern("^\\d{3}-\\d{3}$");
        if(std::regex_match(meetingId_, pattern)) {
            if (signalState != State::LOGIN_ON) {
                E_LOG("please login first");
                return false;
            }
            this->meetingId = meetingId_;

            if (audioEngine == nullptr) {
                audioEngine = std::make_shared<rtcAudioEngine>();
            }
            if (videoEngine == nullptr) {
                videoEngine = std::make_shared<RTCVideoEngine>();
            }
            InitializePeerConnection();

            peer_connection_->CreateOffer(this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());

            //std::thread threadGetdevInfo(&RtcConnectEngine::getDevList, this);
            //getDevInfoThread = std::move(threadGetdevInfo);
        }
        else{
            E_LOG("meetingId error");
            return false;
        }

        return true;
    }

    bool RtcConnectEngine::exitMeeting(){
        Message byeReq;
        byeReq.set_from(userInfo.userId);
        byeReq.set_to(meetingId);
        byeReq.set_cseq(cseq++);
        byeReq.set_call_id(std::to_string(rand()));
        byeReq.set_meth("BYE");
        byeReq.set_isresponse(false);
        D_LOG("bye Req: {}", byeReq.js.dump(4));
        oatpp::String js = oatpp::String(byeReq.js.dump());
        sendSocket(js);

        screenTrackInterface.release();
        videoEngine->close();
        audioEngine->close();
        return true;
    }

    bool RtcConnectEngine::openCamera() {
        videoEngine->switchCamera(true);

        return true;
    }

    bool RtcConnectEngine::closeCamera() {
        videoEngine->switchCamera(false);
        return true;
    }

    bool RtcConnectEngine::openMicphone() {
        Message openMicReq;
        openMicReq.set_from(userInfo.userId);
        openMicReq.set_to(meetingId);
        openMicReq.set_cseq(cseq++);
        openMicReq.set_call_id(std::to_string(rand()));
        openMicReq.set_meth("INFO");
        openMicReq.set_signal("21");
        openMicReq.set_isresponse(false);
        D_LOG("openMic Req: {}", openMicReq.js.dump(4));
        oatpp::String js = oatpp::String(openMicReq.js.dump());
        sendSocket(js);
        bool ret = audioEngine->setMicrophone(true);

        return ret;
    }

    bool RtcConnectEngine::closeMicphone() {
        Message closeMicReq;
        closeMicReq.set_from(userInfo.userId);
        closeMicReq.set_to(meetingId);
        closeMicReq.set_cseq(cseq++);
        closeMicReq.set_call_id(std::to_string(rand()));
        closeMicReq.set_meth("INFO");
        closeMicReq.set_signal("20");
        closeMicReq.set_isresponse(false);
        D_LOG("closeMic Req: {}", closeMicReq.js.dump(4));
        oatpp::String js = oatpp::String(closeMicReq.js.dump());
        sendSocket(js);

        bool ret = audioEngine->setMicrophone(false);

        return ret;
    }

    bool RtcConnectEngine::openScreenShare() {
        Message screenShareReq;
        screenShareReq.set_from(userInfo.userId);
        screenShareReq.set_to(meetingId);
        screenShareReq.set_cseq(cseq++);
        screenShareReq.set_call_id(std::to_string(rand()));
        screenShareReq.set_meth("INFO");
        screenShareReq.set_signal("31");
        screenShareReq.set_isresponse(false);
        D_LOG("openScreenShare Req: {}", screenShareReq.js.dump(4));
        oatpp::String js = oatpp::String(screenShareReq.js.dump());
        sendSocket(js);

        bool ret = videoEngine->switchScreen(true);

        return ret;
    }

    bool RtcConnectEngine::closeScreenShare() {
        Message screenShareReq;
        screenShareReq.set_from(userInfo.userId);
        screenShareReq.set_to(meetingId);
        screenShareReq.set_cseq(cseq++);
        screenShareReq.set_call_id(std::to_string(rand()));
        screenShareReq.set_meth("INFO");
        screenShareReq.set_signal("30");
        screenShareReq.set_isresponse(false);
        D_LOG("closeScreenShare Req: {}", screenShareReq.js.dump(4));
        oatpp::String js = oatpp::String(screenShareReq.js.dump());
        sendSocket(js);

        bool ret = videoEngine->switchScreen(false);

        return ret;
    }

    void RtcConnectEngine::setMicphone(int devId) {
        audioEngine->SetRecordingDevices(devId);
    }

    void RtcConnectEngine::setMicphoneVolume(int val) {
        audioEngine->setMicrophoneVolume(val);
    }

    void RtcConnectEngine::setSpeaker(int devId) {
        audioEngine->SetPlayoutDevices(devId);
    }


//
// signaling virtual func
//
    void RtcConnectEngine::onOK(Message resp) {
        if (resp.cmeth() == "REGISTER") {
            signalState = State::LOGIN_ON;
            OnLoginSuccess(resp.to());
        }
        else if (resp.cmeth() == "INVITE") {
            remoteJsep = resp.sdp();
            setRemote(remoteJsep);
            if(resp.timePoint() == -1) OnJoinMeetingSuccess(seeker::time::currentTime());
            else OnJoinMeetingSuccess(resp.timePoint());
        }
        else if (resp.cmeth() == "BYE") {
            meetingId = "";
            stopCv.notify_one();
        }
    }

    void RtcConnectEngine::onTrying(Message resp) {
        signalState = State::TRYING;
    }

    void RtcConnectEngine::onRinging(Message resp) {
        signalState = State::RINGING;
        D_LOG("iceQue.size: {}", iceQue.size());
        std::unique_lock<std::mutex> lock(queueMutex);
        for(int i = 0; i <= iceQue.size(); i++) {
            auto candidate = iceQue.front();
            sendTrickle(candidate);
            iceQue.pop();
        }
        lock.unlock();
        D_LOG("iceQue.size: {}", iceQue.size());
    }

    void RtcConnectEngine::onUnauthorized(Message resp) {
        if (resp.cmeth() == "REGISTER") {
            OnLoginFailure();
            // 清空用户信息
            userInfo.userId = "";
            userInfo.password = "";
        }
        else if (resp.cmeth() == "INVITE") {
            OnJoinMeetingFailure();
            this->meetingId = "";
        }
        else if(resp.cmeth() == "BYE"){
            E_LOG("exitMeeting error");
        }
    }


//
// PeerConnectionObserver implementation.
//
    void RtcConnectEngine::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
                                   const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>> &streams) {
        D_LOG("OnAddTrack");
        OnReceiveTrack(receiver);
    }

    void RtcConnectEngine::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
        D_LOG("OnRemoveTrack");
        RTC_LOG(LS_INFO) << __FUNCTION__ << " " << receiver->id();
    }

    void RtcConnectEngine::OnIceCandidate(const webrtc::IceCandidateInterface *candidate) {
        D_LOG("OnIceCandidate");
        RTC_LOG(LS_INFO) << __FUNCTION__ << " " << candidate->sdp_mline_index();
        peer_connection_->AddIceCandidate(candidate);

        if(signalState != State::RINGING){
            std::lock_guard<std::mutex> lock(queueMutex);
            iceQue.push(candidate);
        }
        else{
            sendTrickle(candidate);
        }
    }

    void RtcConnectEngine::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
        D_LOG("OnIceGatheringChange");
        sendCandidateDone = true;
        if (new_state == webrtc::PeerConnectionInterface::kIceGatheringComplete) {
            D_LOG("send trickle complete");
//            sendTrickleComplete();
        }
    }

//
// CreateSessionDescriptionObserver implementatOnPaintion.
//
    void RtcConnectEngine::OnSuccess(webrtc::SessionDescriptionInterface *desc) {
        I_LOG("onSuccess");
        peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create().get(), desc);

        desc->ToString(&localJsep);

        Message inviteReq;
        inviteReq.set_from(userInfo.userId);
        inviteReq.set_to(meetingId);
        inviteReq.set_cseq(cseq++);
        inviteReq.set_call_id(std::to_string(rand()));
        inviteReq.set_sdp(localJsep);
        inviteReq.set_meth("INVITE");
        inviteReq.set_isresponse(false);
        I_LOG("joinMeeting Req: {}", inviteReq.js.dump(4));
        oatpp::String inviteJson = oatpp::String(inviteReq.js.dump());
        sendSocket(inviteJson);

        signalState = State::CALLING;

        Json::StreamWriterBuilder factory;
    }

    void RtcConnectEngine::OnFailure(webrtc::RTCError error) {
        D_LOG("OnFailure");
        RTC_LOG(LS_ERROR) << ToString(error.type()) << ": " << error.message();
    }

//
//rtc::VideoSinkInterface<webrtc::VideoFrame>
//
    void RtcConnectEngine::OnFrame(const webrtc::VideoFrame &frame) {

        std::unique_lock<std::mutex> lk(onFrameMtx);
        rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(frame.video_frame_buffer()->ToI420());
        if (image_ == nullptr) {
            image_.reset(new uint8_t[buffer->width() * buffer->height() * 4]);
        }

        RTC_DCHECK(image_.get() != NULL);
        libyuv::I420ToARGB(buffer->DataY(), buffer->StrideY(), buffer->DataU(),
                           buffer->StrideU(), buffer->DataV(), buffer->StrideV(),
                           image_.get(),
                           buffer->width() * 4,
                           buffer->width(), buffer->height());

        return;
    }


//
// private
//
    void RtcConnectEngine::socketTask(const std::shared_ptr<oatpp::websocket::WebSocket> &websocket) {
        try {
            websocket->listen();
        }
        catch (...) {
            E_LOG("websocket->listen error");
        }
    }

    void RtcConnectEngine::keepalive() {
        while (!threadDestroy) {
            D_LOG("send keepalive");
            Message keepaliveReq;
            keepaliveReq.set_from(userInfo.userId);
            keepaliveReq.set_to(userInfo.userId);
            keepaliveReq.set_meth("Heartbeat");
            D_LOG("keepalive Req: {}", keepaliveReq.js.dump(4));
            oatpp::String keepaliveJson = oatpp::String(keepaliveReq.js.dump());
            sendSocket(keepaliveJson);
            Sleep(2000);
        }
    }

    void RtcConnectEngine::sendSocket(oatpp::String js) {
        std::unique_lock<std::mutex> lk(socketMtx);
        signalingSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
    }

    bool RtcConnectEngine::InitializePeerConnection() {
        RTC_DCHECK(!peer_connection_factory_);
        RTC_DCHECK(!peer_connection_);

        rtc::scoped_refptr<webrtc::AudioDeviceModule> adm = audioEngine->InitAdm();

        if (!signaling_thread_.get()) {
            signaling_thread_ = rtc::Thread::CreateWithSocketServer();
            signaling_thread_->Start();
        }
        peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
                nullptr /* network_thread */, nullptr /* worker_thread */,
                signaling_thread_.get() /* signal thread */, adm /* rtc_audio_engine_adm */,
                //signaling_thread_.get() /* signal thread */, nullptr /* rtc_audio_engine_adm */,
                webrtc::CreateBuiltinAudioEncoderFactory(),
                webrtc::CreateBuiltinAudioDecoderFactory(),
                std::make_unique<webrtc::VideoEncoderFactoryTemplate<webrtc::OpenH264EncoderTemplateAdapter>>(),
                std::make_unique<webrtc::VideoDecoderFactoryTemplate<webrtc::OpenH264DecoderTemplateAdapter>>(),
                nullptr /* audio_mixer */, nullptr /* audio_processing */);
        if (!peer_connection_factory_) {
            E_LOG("error");
            return false;
        }

        if (!CreatePeerConnection()) {
            E_LOG("error");
            return false;
        }
        I_LOG("create pc finish");
        AddTracks();
        I_LOG("InitializePeerConnection finish");

        return true;
    }

    void RtcConnectEngine::AddTracks() {
        D_LOG("AddTracks");
        if (!peer_connection_->GetSenders().empty()) {
            return;  // Already added tracks.
        }

        audioEngine->AddAudioTracks(peer_connection_factory_, peer_connection_);

        rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_, screen_track_;
        videoEngine->addVideoTrack(peer_connection_factory_, peer_connection_, video_track_);
        //videoEngine->addScreenTrack(peer_connection_factory_, peer_connection_, screen_track_);
        //screenTrackInterface = screen_track_;

        videoEngine->switchCamera(false);
        //videoEngine->switchScreen(false);
    }

    bool RtcConnectEngine::CreatePeerConnection() {
        RTC_DCHECK(peer_connection_factory_);
        RTC_DCHECK(!peer_connection_);

        webrtc::PeerConnectionInterface::RTCConfiguration config;
        config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
        webrtc::PeerConnectionInterface::IceServer server;
        server.uri = GetPeerConnectionString();
        config.servers.push_back(server);

        webrtc::PeerConnectionDependencies pc_dependencies(this);
        auto error_or_peer_connection = peer_connection_factory_->CreatePeerConnectionOrError(config,
                                                                                              std::move(
                                                                                                      pc_dependencies));
        if (error_or_peer_connection.ok()) {
            peer_connection_ = std::move(error_or_peer_connection.value());
        }
        return peer_connection_ != nullptr;
    }

    void RtcConnectEngine::sendTrickle(const webrtc::IceCandidateInterface *candidate) {
        Message trickleReq;
        trickleReq.set_from(userInfo.userId);
        trickleReq.set_to(meetingId);
        trickleReq.set_cseq(cseq++);
        trickleReq.set_call_id(std::to_string(rand()));
        trickleReq.set_meth("TRICKLE");
        trickleReq.set_sdpMLineIndex(candidate->sdp_mline_index());
        trickleReq.set_sdpMid(candidate->sdp_mid());

        std::string candidateString;
        if (!candidate->ToString(&candidateString)) {
            E_LOG("Failed to serialize candidate");
            return;
        }
        trickleReq.set_candidate(candidateString);
        trickleReq.set_isresponse(false);
        D_LOG("trickle Req: {}", trickleReq.js.dump(4));
        oatpp::String js = oatpp::String(trickleReq.js.dump());
        sendSocket(js);
    }

    void RtcConnectEngine::sendTrickleComplete() {
        Message trickleReq;
        trickleReq.set_from(userInfo.userId);
        trickleReq.set_to(meetingId);
        trickleReq.set_cseq(cseq++);
        trickleReq.set_call_id(std::to_string(rand()));
        trickleReq.set_meth("TRICKLE");
        trickleReq.set_completed(true);
        trickleReq.set_isresponse(false);
        D_LOG("trickle complete Req: {}", trickleReq.js.dump(4));
        oatpp::String js = oatpp::String(trickleReq.js.dump());
        sendSocket(js);
    }

    void RtcConnectEngine::setRemote(std::string jsep) {
        std::unique_ptr<webrtc::SessionDescriptionInterface> session_description = webrtc::CreateSessionDescription(
                webrtc::SdpType::kAnswer, jsep);
        peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create().get(),
                                               session_description.release());
    }

    void RtcConnectEngine::getDevList() {
        while (!threadDestroy) {
            if (audioEngine) {
                audioEngine->GetRecordingDevices(micList);
                if (micList.size() != micListSize) {
                    OnAudioInputDevInfo(micList);
                    micListSize = micList.size();
                }

                audioEngine->GetPlayoutDevices(speakerList);
                if (speakerList.size() != speakerListSize) {
                    OnAudioOutputDevInfo(speakerList);
                    speakerListSize = speakerList.size();
                }
            }
            if (videoEngine) {
                videoEngine->getCameraMap(camList);
                if (camList.size() != camListSize) {
                    OnVideoInputDevInfo(camList);
                    camListSize = camList.size();
                }

                videoEngine->getScreenMap(screenList);
                if (screenList.size() != screenListSize) {
                    OnScreenInfo(screenList);
                    screenListSize = screenList.size();
                }

                videoEngine->getWinMap(windowList);
                if (windowList.size() != windowListSize) {
                    OnWindowInfo(windowList);
                    windowListSize = windowList.size();
                }
            }
            Sleep(1000);
        }
    }

}
