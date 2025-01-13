#include "screenCapturer.h"

ScreenCapturer::ScreenCapturer() {

}

void ScreenCapturer::startCapturer() {
  auto options = webrtc::DesktopCaptureOptions::CreateDefault();
  options.set_allow_directx_capturer(true);
  //capturer_ = webrtc::DesktopCapturer::CreateScreenCapturer(options);
  capturer_ = webrtc::DesktopCapturer::CreateWindowCapturer(options);
  capturer_->Start(this);
  I_LOG("startCapturer");
  working = true;
  std::thread captureTh(&ScreenCapturer::captureThread, this);
  captureTh.detach();
}

void ScreenCapturer::setScreen(uint8_t id) {
  //capturer_->SelectSource(id);
  webrtc::DesktopCapturer::SourceList sources;
  capturer_->GetSourceList(&sources);
  for (const auto& source : sources) {
    I_LOG("Source ID:{}, Title:{}", source.id, source.title);
  }
  capturer_->SelectSource(sources[appNum].id);
  I_LOG("sources.size={}, appNum={},Source ID:{}, Title:{}", sources.size(), appNum, sources[appNum].id, sources[appNum].id);
  if (appNum < sources.size()-1)
    appNum++;
  else appNum = 1;
  
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
  //isOnResult = true;
  if (result != webrtc::DesktopCapturer::Result::SUCCESS)
    return;
  int width = frame->size().width();
  int height = frame->size().height();
  //I_LOG("w:{} h:{}", width, height);

  if (!i420_buffer_.get() ||
    i420_buffer_->width() * i420_buffer_->height() < width * height) {
    i420_buffer_ = webrtc::I420Buffer::Create(width, height);
  }
  libyuv::ConvertToI420(frame->data(), 0, i420_buffer_->MutableDataY(),
    i420_buffer_->StrideY(), i420_buffer_->MutableDataU(),
    i420_buffer_->StrideU(), i420_buffer_->MutableDataV(),
    i420_buffer_->StrideV(), 0, 0, width, height, width,
    height, libyuv::kRotate0, libyuv::FOURCC_ARGB);
  OnFrame(webrtc::VideoFrame(i420_buffer_, 0, 0, webrtc::kVideoRotation_0)); 
  //frame.reset();
  isOnResult = false;
}


void ScreenCapturer::CaptureFrame() {
  capturer_->CaptureFrame();
}

void ScreenCapturer::captureThread() {
  while (working) {
    if (!isOnResult)
      CaptureFrame();
    Sleep(1);
  }
}