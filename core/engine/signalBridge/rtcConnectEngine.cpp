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

    RtcConnectEngine::RtcConnectEngine() {
        oatpp::base::Environment::init();
    }

    RtcConnectEngine::~RtcConnectEngine() {
        threadDestroy = true;
        if (keepaliveThread.joinable()) {
            keepaliveThread.join();
            I_LOG("keepaliveThread.join");
        }

        if(signalingSocket) {
            signalingSocket->sendClose();
            signalingSocket->stopListening();
            signalingSocket = nullptr;
            I_LOG("signalingSocket close");
        }

        if (listenerThread.joinable()) {
            listenerThread.join();
            I_LOG("listenerThread.join");
        }

        audioEngine = nullptr;
        videoEngine = nullptr;
        peer_connection_ = nullptr;
        peer_connection_factory_ = nullptr;
        I_LOG("audioEngine/videoEngine/peer_connection_/peer_connection_factory_ = nullptr");

        oatpp::base::Environment::destroy();
    }

    bool RtcConnectEngine::connect(std::string signalIp_, uint16_t signalPort_) {
        I_LOG("in RtcConnectEngine::connect");
        try {
            auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({signalIp_, signalPort_});
            I_LOG("signaling addr: {}:{}", signalIp_, signalPort_);

            auto connector = oatpp::websocket::Connector::createShared(connectionProvider);
            auto connection = connector->connect("/connectWS");
            signalInfo.signalIp = signalIp_;
            signalInfo.signalPort = signalPort_;
            I_LOG("signaling ws connected done!");

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
            E_LOG("RtcConnectEngine::connect catch error");
            return false;
        }
        I_LOG("RtcConnectEngine::connect done");
        return true;
    }

    bool RtcConnectEngine::login(std::string userId_, std::string password_) {
        I_LOG("in RtcConnectEngine::login");
        if (!userId_.empty() && !password_.empty()) {
            Message loginReq;
            loginReq.set_userid(userId_);
            loginReq.set_password(password_);
            loginReq.set_cseq(cseq++);
            loginReq.set_meth("REGISTER");
            loginReq.set_isresponse(false);
            I_LOG("login Req: {}", loginReq.js.dump(4));
            oatpp::String loginJson = oatpp::String(loginReq.js.dump());
            sendSocket(loginJson);

            userInfo.userId = userId_;
            userInfo.password = password_;
            I_LOG("RtcConnectEngine::login userId: {} password: {}", userInfo.userId, userInfo.password);
        } else {
            E_LOG("userId/password is empty");
            return false;
        }

        I_LOG("RtcConnectEngine::login done");
        return true;
    }

    void RtcConnectEngine::logout() {
        Message logoutReq;
        logoutReq.set_userid(userInfo.userId);
        logoutReq.set_cseq(cseq++);
        logoutReq.set_meth("LOGOUT");
        logoutReq.set_isresponse(false);
        I_LOG("logout Req: {}", logoutReq.js.dump(4));
        oatpp::String logoutJson = oatpp::String(logoutReq.js.dump());
        sendSocket(logoutJson);
    }

    bool RtcConnectEngine::createMeeting(VideoMcu videoMcu_, AudioMcu audioMcu_, VideoCodecType videoCodecType_, AudioCodecType audioCodecType_) {
        this->videoMcu = videoMcu_;
        this->audioMcu = audioMcu_;
        this->videoCodecType = videoCodecType_;
        this->audioCodecType = audioCodecType_;
        I_LOG("videoMcu: {} audioMcu: {} videoCodecType: {} audioCodecType: {}", videoMcu, audioMcu, videoCodecType, audioCodecType);

        if(videoMcu == 0){
            if(audioMcu == 0){
                mcu = 0;
            }
            else if(audioMcu == 1){
                mcu = 1;
            }
        }
        else if(videoMcu == 1){
            if(audioMcu == 0){
                mcu = 2;
            }
            else if(audioMcu == 1){
                mcu = 3;
            }
        }

        if (audioEngine == nullptr) {
            I_LOG("init audioEngine");
            audioEngine = std::make_shared<rtcAudioEngine>();
            I_LOG("init audioEngine done");
        }
        if (videoEngine == nullptr) {
            I_LOG("init videoEngine");
            videoEngine = std::make_shared<RTCVideoEngine>();
            I_LOG("init videoEngine done");
        }
        I_LOG("createMeeting videoType: {} audioType: {}", videoCodecType, audioCodecType);
        InitializePeerConnection();

        peer_connection_->CreateOffer(this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
        audioEngine->setMicrophoneVolume(50);


        return true;
    }

    bool RtcConnectEngine::joinMeeting(std::string meetingId_) {
        I_LOG("in RtcConnectEngine::joinMeeting");
        if (userInfo.userId == "" || userInfo.password == "") {
            E_LOG("please login first");
            return false;
        }
        this->meetingId = meetingId_;
        Message queryReq;
        queryReq.set_from(userInfo.userId);
        queryReq.set_to(meetingId);
        queryReq.set_meth("QUERY");
        queryReq.set_cseq(cseq++);
        queryReq.set_call_id(std::to_string(rand()));
        I_LOG("query Req: {}", queryReq.js.dump(4));
        oatpp::String queryJson = oatpp::String(queryReq.js.dump());
        sendSocket(queryJson);

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
        I_LOG("bye Req: {}", byeReq.js.dump(4));
        oatpp::String js = oatpp::String(byeReq.js.dump());
        sendSocket(js);

        screenTrackInterface.release();
        videoEngine->close();
        audioEngine->close();

        meetingId.clear();
        videoCodecType = -1;
        audioCodecType = -1;
        videoMcu = -1;
        audioMcu = -1;
        peer_connection_ = nullptr;
        peer_connection_factory_ = nullptr;
        I_LOG("exit meeting");
        return true;
    }

    bool RtcConnectEngine::openCamera() {
        videoEngine->setVideoBitrate(0.9);
        videoEngine->switchCamera(true);

        return true;
    }

    bool RtcConnectEngine::closeCamera() {
        videoEngine->setVideoBitrate(0.2);
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
        I_LOG("openMic Req: {}", openMicReq.js.dump(4));
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
        I_LOG("closeMic Req: {}", closeMicReq.js.dump(4));
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
        I_LOG("openScreenShare Req: {}", screenShareReq.js.dump(4));
        oatpp::String js = oatpp::String(screenShareReq.js.dump());
        sendSocket(js);

        videoEngine->setVideoBitrate(1.6);
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
        I_LOG("closeScreenShare Req: {}", screenShareReq.js.dump(4));
        oatpp::String js = oatpp::String(screenShareReq.js.dump());
        sendSocket(js);

        videoEngine->setVideoBitrate(0.9);
        bool ret = videoEngine->switchScreen(false);

        return ret;
    }

    void RtcConnectEngine::setCamera(int devId) {
        videoEngine->setVideoBitrate(0.9);
        videoEngine->setCamera(devId);
        I_LOG("setCamera done");
    }

    void RtcConnectEngine::setScreen(int devId) {
        videoEngine->setVideoBitrate(1.6);
        videoEngine->setScreenCapture(devId);
        I_LOG("setScreen done");
    }

    void RtcConnectEngine::setWindow(int devId) {
        videoEngine->setVideoBitrate(1.6);
        videoEngine->setWindowCapture(devId);
        I_LOG("setWindow done");
    }

    void RtcConnectEngine::setMicphone(int devId) {
        audioEngine->ReplacePlayoutDevices(devId);
        I_LOG("setMicphone done");
    }

    void RtcConnectEngine::setMicphoneVolume(int val) {
        audioEngine->setMicrophoneVolume(val);
        I_LOG("setMicphoneVolume done");
    }

    void RtcConnectEngine::setSpeaker(int devId) {
        audioEngine->ReplacePlayoutDevices(devId);
        I_LOG("setSpeaker done");
    }

    void RtcConnectEngine::getAudioInputDevInfo(std::map<int16_t, std::string>& list) {
        I_LOG("in RtcConnectEngine::getAudioInputDevInfo");
        if(audioEngine){
            audioEngine->GetRecordingDevices(micList);
            if(micList.size() == 0){
                E_LOG("no AudioInputDevInfo");
                return;
            }
            else{
                list.swap(micList);
            }
        }
        else{
            E_LOG("audioEngine is null");
        }
    }

    void RtcConnectEngine::getAudioOutputDevInfo(std::map<int16_t, std::string>& list) {
        I_LOG("in RtcConnectEngine::getAudioOutputDevInfo");
        if(audioEngine){
            audioEngine->GetPlayoutDevices(speakerList);
            if(speakerList.size() == 0){
                E_LOG("no AudioOutputDevInfo");
                return;
            }
            else{
                list.swap(speakerList);
            }
        }
        else{
            E_LOG("audioEngine is null");
        }
    }

    void RtcConnectEngine::getVideoInputDevInfo(std::map<int16_t, std::string>& list) {
        I_LOG("in RtcConnectEngine::getVideoInputDevInfo");
        if(videoEngine){
            videoEngine->getCameraMap(camList);
            if(camList.size() == 0){
                E_LOG("no VideoInputDevInfo");
                return;
            }
            else{
                list.swap(camList);
            }
        }
        else{
            E_LOG("videoEngine is null");
        }
    }

    void RtcConnectEngine::getScreenInfo(std::map<int, std::string>& list) {
        I_LOG("in RtcConnectEngine::getScreenInfo");
        if(videoEngine){
            videoEngine->getScreenMap(screenList);
            if(screenList.size() == 0){
                E_LOG("no ScreenInfo");
                return;
            }
            else{
                list.swap(screenList);
            }
        }
        else{
            E_LOG("videoEngine is null");
        }
    }

    void RtcConnectEngine::getWindowInfo(std::map<int, std::string>& list) {
        I_LOG("in RtcConnectEngine::getWindowInfo");
        if(videoEngine){
            videoEngine->getWinMap(windowList);
            if(windowList.size() == 0){
                E_LOG("no WindowInfo");
                return;
            }
            else{
                list.swap(windowList);
            }
        }
        else{
            E_LOG("videoEngine is null");
        }
    }


//
// signaling virtual func
//
    void RtcConnectEngine::onOK(Message resp) {
        if (resp.cmeth() == "REGISTER") {
            I_LOG("REGISTER onOK");
            signalState = State::LOGIN_ON;
            OnLoginSuccess(userInfo.userId);
        }
        else if(resp.cmeth() == "LOGOUT"){
            I_LOG("LOGOUT onOK");
            signalState = State::NONE;
            OnLogoutSuccess();
        }
        else if(resp.cmeth() == "QUERY"){
            I_LOG("QUERY onOK");
            std::regex pattern("^\\d{3}-\\d{3}$");
            if(std::regex_match(meetingId, pattern)) {
                if (audioEngine == nullptr) {
                    I_LOG("init audioEngine");
                    audioEngine = std::make_shared<rtcAudioEngine>();
                    I_LOG("init audioEngine done");
                }
                if (videoEngine == nullptr) {
                    I_LOG("init videoEngine");
                    videoEngine = std::make_shared<RTCVideoEngine>();
                    I_LOG("init videoEngine done");
                }


                if(resp.videoformat() == "H264"){
                    this->videoCodecType = VideoCodecType::H264;
                }
                else if(resp.videoformat() == "VP9"){
                    this->videoCodecType = VideoCodecType::VP9;
                }

                if(resp.audioformat() == "PCMA"){
                    this->audioCodecType = AudioCodecType::PCMA;
                }
                else if(resp.audioformat() == "OPUS"){
                    this->audioCodecType = AudioCodecType::OPUS;
                }

                I_LOG("joinMeeting videoType: {} audioType: {}", videoCodecType, audioCodecType);
                InitializePeerConnection();

                peer_connection_->CreateOffer(this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
                audioEngine->setMicrophoneVolume(50);
            }

        }
        else if (resp.cmeth() == "INVITE") {
            I_LOG("INVITE onOK");
            signalState = State::MEETING;
            remoteJsep = resp.sdp();
            setRemote(remoteJsep);
            if(resp.timePoint() == -1){
                if(meetingId.empty()){
                    meetingId = resp.meetingId();
                    OnCreateMeetingSuccess(meetingId, seeker::Time::currentTime());
                }
                OnJoinMeetingSuccess(seeker::Time::currentTime());
            }
            else{
                if(meetingId.empty()){
                    meetingId = resp.meetingId();
                    OnCreateMeetingSuccess(meetingId, resp.timePoint());
                }
                OnJoinMeetingSuccess(resp.timePoint());
            }
        }
        else if (resp.cmeth() == "INFO" && resp.signal() == "31") {
            I_LOG("INFO 31 onOK");
            videoEngine->requestKeyFrame();
        }
        else if (resp.cmeth() == "BYE") {
            I_LOG("BYE onOK");
            meetingId.clear();
        }
    }

    void RtcConnectEngine::onTrying(Message resp) {
        I_LOG("onTrying");
        signalState = State::TRYING;
    }

    void RtcConnectEngine::onRinging(Message resp) {
        I_LOG("onRinging");
        signalState = State::RINGING;
        if(localJsep != "unknown") {
            setLocal(localJsep);
        }
        else{
            E_LOG("local jsep unknown");
        }
//        I_LOG("iceQue.size: {}", iceQue.size());
//        std::unique_lock<std::mutex> lock(queueMutex);
//        for(int i = 0; i <= iceQue.size(); i++) {
//            I_LOG("iceQue[{}]", i);
//            auto candidate = iceQue.front();
//            I_LOG("get candidate");
//            sendTrickle(candidate);
//            I_LOG("send iceQue candidate");
//            iceQue.pop();
//        }
//        lock.unlock();
//        I_LOG("iceQue.size: {}", iceQue.size());
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
            this->meetingId.clear();
        }
        else if(resp.cmeth() == "BYE"){
            E_LOG("exitMeeting error");
        }
    }

    void RtcConnectEngine::onHeartbeatResp() {
        noHeartbeatRespTime = 0;
    }

    void RtcConnectEngine::onCancel(Message resp) {
        if(resp.cmeth() == "INVITE") {
            meetingId.clear();
            videoCodecType = -1;
            audioCodecType = -1;
            videoMcu = -1;
            audioMcu = -1;
            peer_connection_ = nullptr;
            peer_connection_factory_ = nullptr;
        }
    }


//
// PeerConnectionObserver implementation.
//
    void RtcConnectEngine::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>> &streams) {
        I_LOG("OnAddTrack");
        OnReceiveTrack(receiver);
    }

    void RtcConnectEngine::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
        I_LOG("OnRemoveTrack");
        RTC_LOG(LS_INFO) << __FUNCTION__ << " " << receiver->id();
    }

    void RtcConnectEngine::OnIceCandidate(const webrtc::IceCandidateInterface *candidate) {
        I_LOG("OnIceCandidate");
        RTC_LOG(LS_INFO) << __FUNCTION__ << " " << candidate->sdp_mline_index();
        peer_connection_->AddIceCandidate(candidate);

//        if(signalState != State::RINGING){
//            std::lock_guard<std::mutex> lock(queueMutex);
//            iceQue.push(candidate);
//        }
//        else{
        sendTrickle(candidate);
//        }
    }

    void RtcConnectEngine::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
        I_LOG("OnIceGatheringChange");
        sendCandidateDone = true;
        if (new_state == webrtc::PeerConnectionInterface::kIceGatheringComplete) {
            I_LOG("send trickle complete");
//            sendTrickleComplete();
        }
    }

//
// CreateSessionDescriptionObserver implementatOnPaintion.
//
    void RtcConnectEngine::OnSuccess(webrtc::SessionDescriptionInterface *desc) {
        I_LOG("onSuccess");
//        peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create().get(), desc);
        std::string sdp;
        desc->ToString(&sdp);
        std::string videoType = "unknown";
        std::string audioType = "unknown";
        if(videoCodecType == VideoCodecType::H264){
            videoType = "H264";
        }
        else if(videoCodecType == VideoCodecType::VP9){
            videoType = "VP9";
        }
        if(audioCodecType == AudioCodecType::PCMA){
            audioType = "PCMA";
        }
        else if(audioCodecType == AudioCodecType::OPUS){
            audioType = "OPUS";
        }
        localJsep = audioEngine->modifySdp(sdp, audioType);
        Message inviteReq;
        if(meetingId.empty()) {
            inviteReq.set_from(userInfo.userId);
            inviteReq.set_cseq(cseq++);
            inviteReq.set_call_id(std::to_string(rand()));
            inviteReq.set_sdp(localJsep);
            inviteReq.set_mcuId(mcu);
            inviteReq.set_videoformat(videoType);
            inviteReq.set_audioformat(audioType);
            inviteReq.set_meth("INVITE");
            inviteReq.set_isresponse(false);
            I_LOG("createMeeting Req: {}", inviteReq.js.dump(4));
        }
        else{
            inviteReq.set_from(userInfo.userId);
            inviteReq.set_to(meetingId);
            inviteReq.set_cseq(cseq++);
            inviteReq.set_call_id(std::to_string(rand()));
            inviteReq.set_sdp(localJsep);
            inviteReq.set_meth("INVITE");
            inviteReq.set_isresponse(false);
            I_LOG("joinMeeting Req: {}", inviteReq.js.dump(4));
        }
        oatpp::String inviteJson = oatpp::String(inviteReq.js.dump());
        sendSocket(inviteJson);

        signalState = State::CALLING;

        Json::StreamWriterBuilder factory;
    }

    void RtcConnectEngine::OnFailure(webrtc::RTCError error) {
        I_LOG("OnFailure");
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
        I_LOG("in RtcConnectEngine::socketTask");
        try {
            websocket->listen();
        }
        catch (...) {
            E_LOG("websocket->listen error");
        }
    }

    void RtcConnectEngine::keepalive() {
        I_LOG("in RtcConnectEngine::keepalive");
        try {
            while (!threadDestroy) {
//                D_LOG("noHeartbeatRespTime: {}", noHeartbeatRespTime);
//                if (noHeartbeatRespTime > 5) {
//                    if(signalingSocket) {
//                        signalingSocket->stopListening();
//                        I_LOG("stopListening");
//                    }
//                    break;
//                }
                D_LOG("send keepalive");
                Message keepaliveReq;
                keepaliveReq.set_from(userInfo.userId);
                keepaliveReq.set_to(userInfo.userId);
                keepaliveReq.set_meth("Heartbeat");
                D_LOG("keepalive Req: {}", keepaliveReq.js.dump(4));
                oatpp::String keepaliveJson = oatpp::String(keepaliveReq.js.dump());
                sendSocket(keepaliveJson);
                noHeartbeatRespTime += 1;
                Sleep(2000);
            }
        }
        catch(...){
            I_LOG("keepalive catch error");
        }
    }

    void RtcConnectEngine::sendSocket(oatpp::String js) {
        try {
            std::unique_lock<std::mutex> lk(socketMtx);
            if(signalingSocket) {
                signalingSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
            }
        }
        catch(...){
            E_LOG("sendSocket catch error");
        }
    }

    bool RtcConnectEngine::InitializePeerConnection() {
        I_LOG("in InitializePeerConnection");
        RTC_DCHECK(!peer_connection_factory_);
        RTC_DCHECK(!peer_connection_);

        rtc::scoped_refptr<webrtc::AudioDeviceModule> adm = audioEngine->InitAdm();

        if (!signaling_thread_.get()) {
            signaling_thread_ = rtc::Thread::CreateWithSocketServer();
            signaling_thread_->Start();
        }

        std::unique_ptr<webrtc::VideoEncoderFactory> video_encoder_factory = nullptr;
        std::unique_ptr<webrtc::VideoDecoderFactory> video_decoder_factory = nullptr;
        videoEngine->getVideoFactory(videoCodecType, video_encoder_factory, video_decoder_factory);
        peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
                nullptr /* network_thread */, nullptr /* worker_thread */,
                signaling_thread_.get() /* signal thread */, adm /* rtc_audio_engine_adm */,
                //signaling_thread_.get() /* signal thread */, nullptr /* rtc_audio_engine_adm */,
                webrtc::CreateBuiltinAudioEncoderFactory(),
                webrtc::CreateBuiltinAudioDecoderFactory(),
                std::move(video_encoder_factory),
                std::move(video_decoder_factory),
                nullptr /* audio_mixer */, nullptr /* audio_processing */);
        if (!peer_connection_factory_) {
            E_LOG("error");
            return false;
        }

        if (!CreatePeerConnection()) {
            E_LOG("error");
            return false;
        }

        AddTracks();
        I_LOG("InitializePeerConnection end");

        return true;
    }

    void RtcConnectEngine::AddTracks() {
        I_LOG("AddTracks");
        if (!peer_connection_->GetSenders().empty()) {
            return;  // Already added tracks.
        }

        audioEngine->AddAudioTracks(peer_connection_factory_, peer_connection_);

        rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_, screen_track_;
        videoEngine->addVideoTrack(peer_connection_factory_, peer_connection_, video_track_);
        videoEngine->addScreenTrack(peer_connection_factory_, peer_connection_, screen_track_);
        screenTrackInterface = screen_track_;

        videoEngine->switchCamera(false);
        videoEngine->switchScreen(false);
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
        I_LOG("trickle Req: {}", trickleReq.js.dump(4));
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
        I_LOG("trickle complete Req: {}", trickleReq.js.dump(4));
        oatpp::String js = oatpp::String(trickleReq.js.dump());
        sendSocket(js);
    }

    void RtcConnectEngine::setLocal(std::string jsep) {
        std::unique_ptr<webrtc::SessionDescriptionInterface> session_description = webrtc::CreateSessionDescription(
                webrtc::SdpType::kOffer, jsep);
        peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create().get(),
                                               session_description.release());
    }

    void RtcConnectEngine::setRemote(std::string jsep) {
        std::unique_ptr<webrtc::SessionDescriptionInterface> session_description = webrtc::CreateSessionDescription(
                webrtc::SdpType::kAnswer, jsep);
        peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create().get(),
                                               session_description.release());
    }

    void RtcConnectEngine::reconnect() {
        if(signalState == State::LOGIN_ON){
            connect(signalInfo.signalIp, signalInfo.signalPort);
            login(userInfo.userId, userInfo.password);
        }
        else if(signalState == State::MEETING){
            connect(signalInfo.signalIp, signalInfo.signalPort);
            login(userInfo.userId, userInfo.password);
            joinMeeting(meetingId);
        }
        else{
            connect(signalInfo.signalIp, signalInfo.signalPort);
        }
    }
}
