#pragma once

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "presenter.h"
#include "mediaclient.h"
#include "rtc_base/thread.h"

namespace alllink {
  class Controller : public webrtc::PeerConnectionObserver,
    public webrtc::CreateSessionDescriptionObserver,
    public SignlingInteractionObserver,
    public VisionCnetralCallback {

  public:
    Controller(SignlingInteractionSystem* client, VisionCnetralBase* vcb);

    void Close() override;

  protected:
    ~Controller();
    bool InitializePeerConnection();
    bool ReinitializePeerConnectionForLoopback();
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
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override {}
    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
    void OnIceConnectionReceivingChange(bool receiving) override {}

    //
    // SignlingInteractionObserver implementation.
    //

    void OnSignedIn() override;

    void OnDisconnected() override;

    void OnPeerConnected(int id, const std::string& name) override;

    void OnPeerDisconnected(int id) override;

    void OnMessageFromPeer(int peer_id, const std::string& message) override;

    void OnMessageSent(int err) override;

    void OnServerConnectionFailure() override;

    //
    // VisionCnetralCallback implementation.
    //

    void StartLogin(const std::string& server, int port) override;

    void DisconnectFromServer() override;

    void ConnectToPeer(int peer_id) override;

    void DisconnectFromCurrentPeer() override;

    void CustomMessageCallback(int msg_id, void* data) override;

    // CreateSessionDescriptionObserver implementation.
    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    void OnFailure(webrtc::RTCError error) override;
  private:

  };
}
