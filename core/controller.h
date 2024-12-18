#pragma once

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "rtc_base/thread.h"

#include "seeker/iniConfig.hpp"
#include "rtcAudioEngine.h"
#include "rtcVideoEngine.h"
#include "janitor.h"
#include "vision.h"
#include "signling.h"
#include "janus.h"


namespace alllink {
  class Controller : public webrtc::PeerConnectionObserver,
    public webrtc::CreateSessionDescriptionObserver,
    public SignlingInteractionObserver,
    public rtcengine::JanitorObserver,
    //public JanusInteractionObserver,
    public VisionCnetralCallback {

  public:
    Controller(SignlingInteractionSystem* client, VisionCnetralBase* vcb, JanusInteractionSystem* janus);

    void Close() override;

  protected:
    ~Controller();
    bool InitializePeerConnection();
    bool CreatePeerConnection();
    void DeletePeerConnection();
    void EnsureStreamingUI();
    void AddTracks();

    //
    // PeerConnectionObserver implementation.
    //

    void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState new_state) override {}
    void OnAddTrack(
      rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
      const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
      streams) override;
    void OnRemoveTrack(
      rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
    void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}
    void OnRenegotiationNeeded() override {}
    void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state) override {}
    void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
    void OnIceConnectionReceivingChange(bool receiving) override {}

    //
    // SignlingInteractionObserver implementation.
    //

    void OnPeerDisconnected(const std::string& id) override;

    void OnMessageFromSignling(const SignInfo& info) override;

    void OnCSMessageFromSignling(const SignInfo& info) override;

    //
    // JanusInteractionObserver implementation.
    //

    //void OnGenerated(const Jsep& tranditional) override;

    //void OnProcessed(const Jsep& jsep) override;

    //
    // JanitorObserver implementation.
    //

    void OnGenerated(const std::string& sdp, const std::string& type) override;
      
    void OnProcessed(const std::string& sdp, const std::string& type) override;

    //void OnReconnect() override;

    //
    // VisionCnetralCallback implementation.
    //

    bool StartLogin(const ServerInfo& server, const UserInfo& user) override;

    void DisconnectFromServer() override;

    bool ConnectToPeer(const std::string& to) override;

    void DisconnectFromCurrentPeer() override;

    void CustomMessageCallback(const Message& msg) override;

    // 
    // CreateSessionDescriptionObserver implementation
    //

    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    void OnFailure(webrtc::RTCError error) override;
  private:
    SignlingInteractionSystem* client_;
    JanusInteractionSystem* janus_;
    VisionCnetralBase* vision_;
    std::unique_ptr<rtc::Thread> signaling_thread_;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection_;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peerConnectionFactory_;
    rtcengine::rtcAudioEngine audioEngine;
    std::map<int16_t, std::string> audioInputDevMap;
    rtcengine::RTCVideoEngine videoEngine;
    //VideoEngine videoEngine;
    std::shared_ptr<rtcengine::Janitor> janusEngine;
    std::string meetId_;
    std::unique_ptr<webrtc::SessionDescriptionInterface> localDesc;
  };
}