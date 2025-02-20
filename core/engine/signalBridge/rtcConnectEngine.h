//
// Created by 姚惠晶 on 2025/1/7.
//

#ifndef SIGNALBRIDGE_RTCCONNECTENGINE_H
#define SIGNALBRIDGE_RTCCONNECTENGINE_H

#pragma once
#include "iostream"
#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "api/create_peerconnection_factory.h"
#include "rtc_base/thread.h"
#include "absl/memory/memory.h"
#include "api/audio/audio_device.h"
#include "api/audio/audio_mixer.h"
#include "api/audio/audio_processing.h"
#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/audio_options.h"
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
#include "oatpp-websocket/WebSocket.hpp"
#include "oatpp-websocket/Connector.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"

#include "seeker/loggerApi.h"
#include "engineListener.h"
#include <condition_variable>
#include <mutex>
#include "wsMessage.hpp"

#include "AudioEngine/rtcAudioEngine.h"
#include "VideoEngine/rtcVideoEngine.h"
#include "defaults.h"

namespace rtcengine{

    template<class K, class V, class dummy_compare, class A>
    using my_workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
    using json = nlohmann::basic_json<my_workaround_fifo_map>;


    class RtcConnectEngine : public ConnectEngineObserver,
        public webrtc::PeerConnectionObserver,
        public webrtc::CreateSessionDescriptionObserver,
        public rtc::VideoSinkInterface<webrtc::VideoFrame> {
    public:
        enum State {
            NONE,

            // ws已连接
            CONNECT_ON = 1,

            // 已登录信令服务器
            LOGIN_ON,

            // 作为主叫生成Offer SDP并发送FORWARD后，等待接收Trying
            CALLING,

            // 作为主叫收到Trying后，继续等待接收Ringing
            TRYING,

            // 作为主叫收到Ringing后，继续等待接收OK，并从中取出SDP
            RINGING,

            // 在会议中
            MEETING
        };

        enum VideoCodecType{
            H264 = 0,
            VP9 = 1
        };

        enum AudioCodecType{
            PCMA = 0,
            OPUS = 1
        };

        struct SignalingInfo{
            std::string signalIp = "";
            uint16_t signalPort = -1;
        };

        struct UserInfo{
            std::string userId = "";
            std::string password = "";
        };


        RtcConnectEngine();
        ~RtcConnectEngine();

        // 连接信令服务器
        bool connect(std::string signalIp, uint16_t signalPort);
        // 用户登录
        bool login(std::string userId, std::string password);
        // 创建会议
        bool createMeeting(int mcu, VideoCodecType videoType_, AudioCodecType audioType_);
        // 加入会议
        bool joinMeeting(std::string meetingId);
        // 退出会议
        bool exitMeeting();
        // 开启摄像头
        bool openCamera();
        // 关闭摄像头
        bool closeCamera();
        // 开启麦克风
        bool openMicphone();
        // 关闭麦克风
        bool closeMicphone();
        // 开启屏幕共享
        bool openScreenShare();
        // 关闭屏幕共享
        bool closeScreenShare();

        void setCamera(int devId);

        void setScreen(int devId);

        void setWindow(int devId);

        void setMicphone(int devId);

        void setMicphoneVolume(int val);

        void setSpeaker(int devId);

        // 获取媒体设备信息
        void getAudioInputDevInfo(std::map<int16_t, std::string>& list);

        void getAudioOutputDevInfo(std::map<int16_t, std::string>& list);

        void getVideoInputDevInfo(std::map<int16_t, std::string>& list);

        void getScreenInfo(std::map<int, std::string>& list);

        void getWindowInfo(std::map<int, std::string>& list);


        //
        // 回调函数
        //
        // 登录成功
        virtual void OnLoginSuccess(std::string userId) = 0;
        // 登陆失败
        virtual void OnLoginFailure() = 0;
        // OnAddTrack, 即入会成功
        virtual void OnReceiveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) = 0;
        // 入会失败
        virtual void OnJoinMeetingSuccess(int64_t timePoint) = 0;
        // 入会失败
        virtual void OnJoinMeetingFailure() = 0;
//        //获取到麦克风设备信息
//        virtual void OnAudioInputDevInfo(std::map<int16_t, std::string> list) = 0;
//        // 获取扬声器设备信息
//        virtual void OnAudioOutputDevInfo(std::map<int16_t, std::string> list) = 0;
//        // 获取摄像头设备信息
//        virtual void OnVideoInputDevInfo(std::map<int16_t, std::string> list) = 0;
//        // 获取屏幕设备信息
//        virtual void OnScreenInfo(std::map<int, std::string> list) = 0;
//        // 获取窗口信息
//        virtual void OnWindowInfo(std::map<int, std::string> list) = 0;

        std::atomic<bool> threadDestroy = false;

    private:
        void socketTask(const std::shared_ptr<oatpp::websocket::WebSocket>& websocket);
        void keepalive();
        void sendSocket(oatpp::String js);
        bool InitializePeerConnection();
        bool CreatePeerConnection();
        void AddTracks();
        void sendTrickle(const webrtc::IceCandidateInterface* candidate);
        void sendTrickleComplete();
        void setLocal(std::string jsep);
        void setRemote(std::string jsep);
        void getDevList();
        void reconnect();

        //
        // signaling virtual func
        //
        virtual void onOK(Message resp) override;
        virtual void onTrying(Message resp) override;
        virtual void onRinging(Message resp) override;
        virtual void onUnauthorized(Message resp) override;
        virtual void onHeartbeatResp() override;


        //
        // PeerConnectionObserver implementation.
        //
        void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {}
        void OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) override;
        void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
        void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}
        void OnRenegotiationNeeded() override {}
        void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override {}
        void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
        void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
        void OnIceConnectionReceivingChange(bool receiving) override {}

        //
        // CreateSessionDescriptionObserver implementatOnPaintion.
        //
        void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
        void OnFailure(webrtc::RTCError error) override;

        //
        //rtc::VideoSinkInterface<webrtc::VideoFrame>
        //
        void OnFrame(const webrtc::VideoFrame& frame) override;


        int cseq = 0;

        std::string meetingId = "unknown";

        std::shared_ptr<oatpp::websocket::WebSocket> signalingSocket = nullptr;

        std::shared_ptr<EngineListener> signalingListener = nullptr;

        std::thread listenerThread;

        std::thread keepaliveThread;

//        std::thread getDevInfoThread;

        std::string localJsep = "unknown";

        std::string remoteJsep = "unknown";

        std::mutex registerMtx;
        std::condition_variable registerCv;

        std::mutex inviteMtx;
        std::condition_variable inviteCv;


        std::mutex stopMtx;
        std::condition_variable stopCv;

        std::mutex onFrameMtx;

        std::mutex socketMtx;

        rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;

        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;

        std::unique_ptr<rtc::Thread> signaling_thread_;

        std::atomic<bool> sendCandidateDone = false;

        std::shared_ptr<rtcAudioEngine> audioEngine = nullptr;

        std::shared_ptr<RTCVideoEngine> videoEngine = nullptr;

        std::unique_ptr<uint8_t[]> image_ = nullptr;

        bool isNotified = false;

        State signalState{ NONE };

        UserInfo userInfo;

        SignalingInfo signalInfo;

//        std::queue<const webrtc::IceCandidateInterface *> iceQue;

        std::mutex queueMutex;

        rtc::scoped_refptr<webrtc::VideoTrackInterface> screenTrackInterface;

        std::map<int16_t, std::string> micList;

        int micListSize = 0;

        std::map<int16_t, std::string> speakerList;

        int speakerListSize = 0;

        std::map<int16_t, std::string> camList;

        int camListSize = 0;

        std::map<int, std::string> screenList;

        int screenListSize = 0;

        std::map<int, std::string> windowList;

        int windowListSize = 0;

        int noHeartbeatRespTime = 0;

        int videoCodecType;

        int audioCodecType;

        int mcu;

    };

}

#endif //SIGNALBRIDGE_RTCCONNECTENGINE_H
