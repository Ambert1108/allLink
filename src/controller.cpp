/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "controller.h"

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
#include "examples/peerconnection/client/defaults.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "p2p/base/port_allocator.h"
#include "pc/video_track_source.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "rtc_base/strings/json.h"
#include "test/vcm_capturer.h"

namespace alllink {

  std::string GetEnvVarOrDefault(const char* env_var_name,
    const char* default_value) {
    std::string value;
    const char* env_var = getenv(env_var_name);
    if (env_var)
      value = env_var;

    if (value.empty())
      value = default_value;

    return value;
  }

  std::string GetPeerConnectionString() {
    return GetEnvVarOrDefault("WEBRTC_CONNECT", "stun:stun.l.google.com:19302");
  }

  std::string GetDefaultServerName() {
    return GetEnvVarOrDefault("WEBRTC_SERVER", "localhost");
  }

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

namespace alllink {
  Controller::Controller(SignlingInteractionSystem* client, VisionCnetralBase* vcb)
  : client_(client), vision_(vcb) {
    client_->registerObserver(this);
    vision_->registerObserver(this);
  }

  void Controller::Close() {

  }

  Controller::~Controller() {

  }

  bool Controller::InitializePeerConnection() {
    peerConnectionFactory_ = webrtc::CreatePeerConnectionFactory(
      nullptr /* network_thread */, nullptr /* worker_thread */,
      nullptr /* signal thread */, nullptr /* default_adm */,
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

    if (!peerConnectionFactory_) {
      E_LOG("[Controller::InitializePeerConnection] Failed to initialize PeerConnectionFactory");
      DeletePeerConnection();
      return false;
    }

    if (!CreatePeerConnection()) {
      E_LOG("[Controller::InitializePeerConnection] CreatePeerConnection failed");
      DeletePeerConnection();
      return false;
    }
    D_LOG("init PeerConnection");
    AddTracks();
    D_LOG("init finish");

    return true;
  }

  bool Controller::CreatePeerConnection() {
    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = GetPeerConnectionString();
    config.servers.push_back(server);

    webrtc::PeerConnectionDependencies pc_dependencies(this);
    auto error_or_peer_connection =
      peerConnectionFactory_->CreatePeerConnectionOrError(
        config, std::move(pc_dependencies));
    if (error_or_peer_connection.ok()) {
      peerConnection_ = std::move(error_or_peer_connection.value());
    }
    return peerConnection_ != nullptr;
  }

  void Controller::DeletePeerConnection() {
    vision_->stopLocalRenderer();
    vision_->stopRemoteRenderer();
    peerConnection_ = nullptr;
    peerConnectionFactory_ = nullptr;
    meetId_.clear();
  }

  void Controller::EnsureStreamingUI() {

  }

  void Controller::AddTracks() {
    if (!peerConnection_->GetSenders().empty()) {
      return;  // 轨道已添加
    }

    // 创建音频轨道
    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
      peerConnectionFactory_->CreateAudioTrack(
        "audio_label",
        peerConnectionFactory_->CreateAudioSource(cricket::AudioOptions())
        .get()));
    // 添加音频轨道到peerConnection
    auto result_or_error = peerConnection_->AddTrack(audio_track, { "stream_id" });
    if (!result_or_error.ok()) {
      E_LOG("Failed to add audio track to PeerConnection:{}", result_or_error.error().message());
    }

    // 寻找本地采集设备
    rtc::scoped_refptr<CapturerTrackSource> video_device = CapturerTrackSource::Create();
    if (video_device) {
      // 创建视频轨道
      rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_(
        peerConnectionFactory_->CreateVideoTrack(video_device, "video_label"));

      // 向视觉控制器添加本地渲染器
      vision_->startLocalRenderer(video_track_.get());

      // 添加视频轨道到peerConnection
      result_or_error = peerConnection_->AddTrack(video_track_, { "stream_id" });
      if (!result_or_error.ok()) {
        E_LOG("Failed to add video track to PeerConnection: {}", result_or_error.error().message());
      }
    }
    else {
      E_LOG("OpenVideoCaptureDevice failed");
    }

    // 通知视觉控制器切换界面至会议画面
    vision_->switchStreamScreen();
  }


  //
  // PeerConnectionObserver implementation.
  //
  void Controller::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
    const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) {

  }

  void Controller::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {

  }

  void Controller::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {

  }


  //
  // SignlingInteractionObserver implementation.
  //

  void Controller::OnSignedIn() {

  }


  void Controller::OnDisconnected() {

  }


  void Controller::OnPeerConnected(int id, const std::string& name) {

  }


  void Controller::OnPeerDisconnected(int id) {

  }


  void Controller::OnMessageFromPeer(int peer_id, const std::string& message) {

  }


  void Controller::OnMessageSent(int err) {

  }


  void Controller::OnServerConnectionFailure() {

  }


  //
  // VisionCnetralCallback implementation.
  //

  bool Controller::StartLogin(const LinkInfo& link, const UserInfo& user) {
    //调用信令接口实现登录
    if (!client_->connectServer(link)) {
      W_LOG("[Controller::StartLogin] link server {}:{} failed", link.serverIp_, link.serverPort_);
      return false;
    }
    if (!client_->login(user)) {
      W_LOG("[Controller::StartLogin] {} login failed", user.id_);
      return false;
    }
    I_LOG("[Controller::StartLogin] login user:{} to {}:{} done", user.id_, link.serverIp_, link.serverPort_);
    return true;
  }


  void Controller::DisconnectFromServer() {

  }


  bool Controller::ConnectToPeer(const std::string& to) {
    //用户触发
    if (peerConnection_.get()) {
      E_LOG("[Controller::ConnectToPeer] Only one call can be established at a time");
      return false;
    }
    if (!InitializePeerConnection()) {
      E_LOG("[Controller::ConnectToPeer] init peer connection failed");
      return false;
    }
    meetId_ = to;
    // 创建offer sdp
    // 生成offer后会通过OnSuccess回调函数传回offer
    // 在OnSuccess函数中触发信令流程
    peerConnection_->CreateOffer(this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    return true;
  }


  void Controller::DisconnectFromCurrentPeer() {

  }


  void Controller::CustomMessageCallback(int msg_id, void* data) {

  }


  // CreateSessionDescriptionObserver implementation.
  void Controller::OnSuccess(webrtc::SessionDescriptionInterface* desc) {

  }

  void Controller::OnFailure(webrtc::RTCError error) {

  }
}