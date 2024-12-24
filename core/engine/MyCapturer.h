#pragma once

#include <api/scoped_refptr.h>
#include <api/video/i420_buffer.h>
#include <modules/desktop_capture/desktop_capturer.h>
#include <modules/desktop_capture/desktop_frame.h>
#include "rtc_base/thread.h"
#include "media/base/adapted_video_track_source.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "pc/peer_connection_message_handler.h"
#include<iostream>
#include<seeker/common.h>
#include<seeker/loggerApi.h>
#include<seeker/logger.h>
#include "rtc_base/thread.h"
//#include "rtc_base/message_handler.h"
#include "rtc_base/thread.h"
#include <modules/desktop_capture/desktop_capture_options.h>
#include <third_party/libyuv/include/libyuv.h>


class MyCapturer : public rtc::AdaptedVideoTrackSource,
  //public rtc::Thread,
  //public rtc::MessageHandler,
  public webrtc::DesktopCapturer::Callback {
public:
  MyCapturer();

  void startCapturer();

  void CaptureFrame();

  bool is_screencast() const override;

  absl::optional<bool> needs_denoising() const override;

  webrtc::MediaSourceInterface::SourceState state() const override;

  bool remote() const override;

  void OnCaptureResult(webrtc::DesktopCapturer::Result result,
    std::unique_ptr<webrtc::DesktopFrame> frame) override;
  //void OnMessage(rtc::Message* msg) override;

  void captureThread();

  bool working = false;

private:
  std::unique_ptr<webrtc::DesktopCapturer> capturer_;
  rtc::scoped_refptr<webrtc::I420Buffer> i420_buffer_;
  bool isOnResult = false;
  //mutable volatile int ref_count_;
};
