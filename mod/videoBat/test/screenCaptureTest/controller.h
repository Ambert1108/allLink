#ifndef EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_
#define EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_
#include<iostream>
#include<seeker/common.h>
#include<seeker/loggerApi.h>
#include<seeker/logger.h>
#include"httplib.h"
#include <string>
#include <chrono>
#include <condition_variable>
#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <optional>
#include <utility>
#include <vector>
#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "api/rtp_sender_interface.h"
#include "api/rtp_parameters.h"
#include "api/video_codecs/video_encoder.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "api/video/video_frame.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
//#include "api/video/video_de"
#include "rtc_base/thread.h"
#include "nlohmann/json.hpp"
#include "nlohmann/fifomap.hpp"
//#include "presenter.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "savequeue.h"
#include "renderer.h"
#include "engine/rtcVideoEngine.h"
#include "engine/screenCapturer.h"
#include <modules/desktop_capture/desktop_capture_options.h>

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

  void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override;
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
  std::string answersdp;
  std::string offid;
  std::shared_ptr< httplib::Client> cli;
  std::unique_ptr<rtc::Thread> signaling_thread_;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
    peer_connection_factory_;

  //wnd
  sf::RenderWindow* wnd = nullptr;
  sf::Event event{};
  std::unique_ptr<VideoRenderer> local_renderer_ = nullptr;
  std::unique_ptr<VideoRenderer> remote_renderer_;
  std::unique_ptr<VideoRenderer> screen_renderer_;
  zx::ThreadSafeQueue<ImageData> remoteImageList{};
  zx::ThreadSafeQueue<ImageData> localImageList{};
  zx::ThreadSafeQueue<ImageData> screenImageList{};
  sf::Texture* localSource = nullptr;
  sf::Texture* remoteSource = nullptr;
  sf::Texture* screenSource = nullptr;
  sf::Sprite localVideo;
  sf::Sprite remoteVideo;
  sf::Sprite screenVideo;
  int wndWidth = 0;
  int wndHeight = 0;

  std::unique_ptr<rtcengine::RTCVideoEngine> ve = nullptr;

private:
  bool create();
  bool attach();
  bool sendmessage();
  bool sendoffersdp(std::string offersdp);
  bool sendtrickle(std::string candidate, std::string sdpMid, int sdpMLineIndex);
  bool sendtrickle();
  void getJanus(std::string transaction, bool sendsdp);
  void getJanu();
  void keeplive();
  //void switchCamera(bool ifOpen);
  void replaceTrack(int index);
  int appNum = 0;

protected:
  void OnPaint();

};

//class MyCustomVideoEncoder : public webrtc::VideoEncoder {
//public:
//  MyCustomVideoEncoder() {
//    // 初始化编码器
//  }
//  
//  // 实现编码器的必要方法
//  int32_t InitEncode(const webrtc::VideoCodec* codec_settings, int32_t number_of_cores, size_t max_payload_size) override {
//    // 初始化编码器设置
//    return WEBRTC_VIDEO_CODEC_OK;
//  }
//
//  int32_t Encode(const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frame_types) override {
//  //int32_t Encode(const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frame_types, const webrtc::CodecSpecificInfo* codec_specific_info, int64_t time_stamp) override {
//    // 编码视频帧
//    return WEBRTC_VIDEO_CODEC_OK;
//  }
//
//  int32_t RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback) override {
//    // 注册编码完成回调
//    return WEBRTC_VIDEO_CODEC_OK;
//  }
//
//  int32_t Release() override {
//    // 释放资源
//    return WEBRTC_VIDEO_CODEC_OK;
//  }
//
//  // 其他必要的实现...
//};
//
//class MyCustomVideoEncoderFactory : public webrtc::VideoEncoderFactory {
//public:
//  MyCustomVideoEncoderFactory() {}
//
//  // 返回支持的编码格式
//  std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override {
//    return { webrtc::SdpVideoFormat("H264"), webrtc::SdpVideoFormat("VP8") }; // 示例支持 H264 和 VP8
//  }
//  
//  // 创建编码器实例
//  std::unique_ptr<webrtc::VideoEncoder> Create(const webrtc::Environment& env, const webrtc::SdpVideoFormat& format) override {
//    if (format.name == "H264") {
//  //    //return rtc::make_ref_counted<MyCustomVideoEncoder>();
//      return std::make_unique<MyCustomVideoEncoder>();
//    }
//    // 处理其他格式...
//    return nullptr;
//  }
//
//  // 其他必要的实现...
//};

//
//class MyVideoEncoder : public webrtc::VideoEncoder {
//public:
//  MyVideoEncoder() {}
//  ~MyVideoEncoder() override {}
//
//  webrtc::VideoEncoder::EncoderInfo GetEncoderInfo() const override {
//    // 返回编码器信息
//    return webrtc::VideoEncoder::EncoderInfo();
//  }
//  
//  void Encode(const webrtc::VideoFrame& frame,
//    const absl::optional<rtc::RtpPacketizer::Options>& options) override {
//    // 检查是否提供了选项
//    if (options) {
//      // 根据 options 执行编码逻辑
//      // 例如，使用 options->max_payload_size 来设置最大负载大小
//      int max_payload_size = options->max_payload_size.value_or(1200); // 默认值
//
//      // 编码逻辑
//      // ...
//    }
//    else {
//      // 如果没有提供选项，使用默认设置
//      // 默认编码逻辑
//      // ...
//    }
//  }
//
//  void SetRateAllocation(const rtc::RtpPacketizer::RateAllocation& allocation,
//    uint32_t framerate) override {
//    // 设置比特率和帧率
//  }
//
//  // 其他必要的方法...
//};
//
//class MyVideoEncoder :public webrtc::VideoEncoder {
//public:
//  MyVideoEncoder() {}
//  ~MyVideoEncoder() override {}
//
//  // 实现 VideoEncoder 接口的方法
//  webrtc::VideoEncoder::EncoderInfo GetEncoderInfo() const override {
//    // 返回编码器信息
//  }
//
//  
//  void Encode(const webrtc::VideoFrame& frame, const webrtc::Optional<rtc::RtpPacketizer::Options>& options) override {
//    // 编码逻辑
//  }
//
//  void SetRateAllocation(const rtc::RtpPacketizer::RateAllocation& allocation, uint32_t framerate) override {
//    // 设置比特率和帧率
//  }
//};

#endif  // EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_