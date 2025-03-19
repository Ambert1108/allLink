#include "screenCapturer.h"

ScreenCapturer::ScreenCapturer() {

}

void ScreenCapturer::startCapturer() {
  std::lock_guard<std::mutex> lock(mutex_);
  auto options = webrtc::DesktopCaptureOptions::CreateDefault();
  options.set_allow_directx_capturer(true);
  options.set_prefer_cursor_embedded(true);
  origin_screen_capturer_ = webrtc::DesktopCapturer::CreateScreenCapturer(options);
  screen_capturer_ = std::make_unique<webrtc::DesktopAndCursorComposer>(std::move(origin_screen_capturer_), options);
  //window_capturer_ = webrtc::DesktopCapturer::CreateWindowCapturer(options);
  //current_capturer_ = screen_capturer_.get();
  //current_capturer_ = window_capturer_.get();
  screen_capturer_->Start(this);
  screen_capturer_->SelectSource(0);
  I_LOG("startCapturer");
  working = true;

  screenThread_ = std::thread(&ScreenCapturer::captureThread, this);

  //std::thread captureTh(&ScreenCapturer::captureThread, this);
  //captureTh.detach();
}

void ScreenCapturer::startWindowCapturer() {
  auto options = webrtc::DesktopCaptureOptions::CreateDefault();
  options.set_allow_directx_capturer(true);
  options.set_prefer_cursor_embedded(true);
  origin_window_capturer_ = webrtc::DesktopCapturer::CreateWindowCapturer(options);
  window_capturer_ = std::make_unique<webrtc::DesktopAndCursorComposer>(std::move(origin_window_capturer_), options);
  //current_capturer_ = screen_capturer_.get();
  //current_capturer_ = window_capturer_.get();
  window_capturer_->Start(this);
  window_capturer_->SelectSource(0);
  I_LOG("startCapturer");
  working = true;
  screenThread_ = std::thread(&ScreenCapturer::captureWindowThread, this);
  //std::thread captureTh(&ScreenCapturer::captureWindowThread, this);
  //captureTh.detach();
}

void ScreenCapturer::setScreen(uint8_t id) {
  //capturer_->SelectSource(id);
  //if (current_capturer_ == window_capturer_.get())
  //  current_capturer_ = screen_capturer_.get();
  webrtc::DesktopCapturer::SourceList sources;
  screen_capturer_->GetSourceList(&sources);
  for (const auto& source : sources) {
    I_LOG("Source ID:{}, Title:{}", source.id, source.title);
  }
  screen_capturer_->SelectSource(id);
  I_LOG("sources.size={}, select id = {}", sources.size(), id);
}

void ScreenCapturer::setWindow(int id) {
  //capturer_->SelectSource(id);
  //if (!window_capturer_) {
  //  I_LOG("window_capturer_ is not initialized");
  //  return;
  //}
  //if (current_capturer_ == screen_capturer_.get()) {
  //  current_capturer_ = window_capturer_.get();
  //}
  webrtc::DesktopCapturer::SourceList sources;
  window_capturer_->GetSourceList(&sources);
  for (const auto& source : sources) {
    I_LOG("Source ID:{}, Title:{}", source.id, source.title);
  }
  // if (id > sources.size() - 1)
   //  id = 1;
  window_capturer_->SelectSource(id);
  I_LOG("sources.size={}, Source ID:{}", sources.size(), id);
}

webrtc::MediaSourceInterface::SourceState ScreenCapturer::state() const {
  return webrtc::MediaSourceInterface::kLive;
}

bool ScreenCapturer::remote() const {
  return false;
}

bool ScreenCapturer::is_screencast() const {
  return true;
}

absl::optional<bool> ScreenCapturer::needs_denoising() const {
  return false;
}

void ScreenCapturer::OnCaptureResult(webrtc::DesktopCapturer::Result result,
  std::unique_ptr<webrtc::DesktopFrame> frame) {
  try {
    //isOnResult = true;
    if (result != webrtc::DesktopCapturer::Result::SUCCESS)
      return;
    int width = frame->size().width();
    int height = frame->size().height();

    int cropWidth = (16 - width % 16);
    if (width % 16 != 0) {
      //  //i420_buffer_.release();
      width = (16 - width % 16) + width;
    }

    if (height % 2) {
      //i420_buffer_.release();
      height = height - 1;
    }
    int stride_y = width;
    int stride_uv = (width + 1) / 2;
    //I_LOG("w:{} h:{} xxx {}", width, height, width * height);
    if (!i420_buffer_.get() || i420_buffer_->width() * i420_buffer_->height() != width * height) {
      i420_buffer_.release();
      i420_buffer_ = webrtc::I420Buffer::Create(width, height, stride_y, stride_uv, stride_uv);
    }
    //I_LOG("xxxxxxxxx w:{} h:{} xxx {}", i420_buffer_->width(), i420_buffer_->height(), i420_buffer_->width() * i420_buffer_->height());

    int ret = libyuv::ARGBToI420(frame->data(), frame->stride(),
      i420_buffer_->MutableDataY(), i420_buffer_->StrideY(),
      i420_buffer_->MutableDataU(), i420_buffer_->StrideU(),
      i420_buffer_->MutableDataV(), i420_buffer_->StrideV(),
      width, height);

    //libyuv::ConvertToI420(frame->data(), 0, i420_buffer_->MutableDataY(),
    //  i420_buffer_->StrideY(), i420_buffer_->MutableDataU(),
    //  i420_buffer_->StrideU(), i420_buffer_->MutableDataV(),
    //  i420_buffer_->StrideV(), 0, 0, width, height, width,
    //  height, libyuv::kRotate0, libyuv::FOURCC_ARGB);
    OnFrame(webrtc::VideoFrame(i420_buffer_, 0, rtc::TimeMillis(), webrtc::kVideoRotation_0));
    //frame.reset();
    isOnResult = false;
  }
  catch (const std::exception& ex) {
    E_LOG("{}", ex.what());
  }
  catch (...) {
    E_LOG("Unknown exception caught");
  }

}


void ScreenCapturer::CaptureFrame() {
  screen_capturer_->CaptureFrame();
}

void ScreenCapturer::CaptureWindowFrame() {
  window_capturer_->CaptureFrame();
}

void ScreenCapturer::captureThread() {
  while (working) {
    if (!isOnResult) {
      CaptureFrame();
    }
    Sleep(1);
  }
}

void ScreenCapturer::captureWindowThread() {
  while (working) {
    if (!isOnResult) {
      CaptureWindowFrame();
    }
    Sleep(1);
  }
}

void ScreenCapturer::stopCapturer() {
  std::lock_guard<std::mutex> lock(mutex_);
  working = false;
  if (screenThread_.joinable()) {
    screenThread_.join();
  }
}