#ifndef EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_
#define EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_
#include "rtcAudioEngine/rtcAudioEngine.h"
#include"utils/httplib.h"
#include"nlohmann/fifo_map.hpp"
#include"nlohmann/json.hpp"
#include "api/audio_codecs/L16/audio_decoder_L16.h"
#include "api/audio_codecs/audio_codec_pair_id.h"
#include "api/audio_codecs/audio_decoder.h"
#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_decoder_factory_template.h"
#include "api/audio_codecs/audio_format.h"
#include "api/audio_codecs/g711/audio_decoder_g711.h"
#include "api/scoped_refptr.h"
template<class K, class V, class dummy_compare, class A>
using my_workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<my_workaround_fifo_map>;

class Conductor : public webrtc::PeerConnectionObserver,
    public webrtc::CreateSessionDescriptionObserver {
public:
    Conductor(std::shared_ptr< httplib::Client> _cli);
    ~Conductor();
    bool InitializePeerConnection();
    bool ReinitializePeerConnectionForLoopback();
    bool CreatePeerConnection();
    void DeletePeerConnection();
    void EnsureStreamingUI();
    void AddTracks();
    void start();
    //
    // PeerConnectionObserver implementation.
    //

    void OnSignalingChange(
        webrtc::PeerConnectionInterface::SignalingState new_state) override {
    }
    void OnAddTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
        const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
        streams) override;
    void OnRemoveTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
    void OnDataChannel(
        rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {
    }
    void OnRenegotiationNeeded() override {}
    void OnIceConnectionChange(
        webrtc::PeerConnectionInterface::IceConnectionState new_state) override {
    }
    void OnIceGatheringChange(
        webrtc::PeerConnectionInterface::IceGatheringState new_state) override;

    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
    void OnIceConnectionReceivingChange(bool receiving) override {}

    // CreateSessionDescriptionObserver implementation.
    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    void OnFailure(webrtc::RTCError error) override;
    int peer_id_;
    bool loopback_;
    long long session_id;
    long long handle_id;
    std::shared_ptr<rtcengine::rtcAudioEngine> rtcaudioEngine;
    std::string answersdp;
    std::string offid;
    std::shared_ptr< httplib::Client> cli;
    std::unique_ptr<rtc::Thread> signaling_thread_;
    std::unique_ptr<rtc::Thread> workerThread;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
        peer_connection_factory_;
    rtc::scoped_refptr<webrtc::AudioDeviceModule> adm;
    rtc::scoped_refptr<webrtc::AudioTrackInterface> at;
    std::unique_ptr<webrtc::TaskQueueFactory> task_queue_factory;
private:
    bool create();
    bool attach();
    bool sendmessage();
    bool sendoffersdp(std::string offersdp);
    bool sendtrickle(std::string candidate,std::string sdpMid,int sdpMLineIndex);
    bool sendtrickle();
    void getJanus(std::string transaction,bool sendsdp);
    void keeplive();
    //rtc::scoped_refptr<webrtc::AudioDeviceModule> InitAdm();
    //void GetAudioDevices(int16_t& recording_num_devices, int16_t& playout_num_devices);
    //bool SetAudioDevices(uint16_t recording_index = 0, uint16_t playout_index = 0);
    //bool ReplaceAudioDevices(uint16_t recording_index = 0, uint16_t playout_index = 0);
};

#endif  // EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_