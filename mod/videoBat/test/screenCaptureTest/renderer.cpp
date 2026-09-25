#include "renderer.h"

VideoRenderer::VideoRenderer(std::function<void()> callback,
  int width,
  int height,
  webrtc::VideoTrackInterface* track_to_render)
  : paint(callback), rendered_track_(track_to_render) {
  ::InitializeCriticalSection(&buffer_lock_);
  I_LOG("1");
  ZeroMemory(&bmi_, sizeof(bmi_));
  bmi_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  I_LOG("2");
  bmi_.bmiHeader.biPlanes = 1;
  bmi_.bmiHeader.biBitCount = 32;
  bmi_.bmiHeader.biCompression = BI_RGB;
  bmi_.bmiHeader.biWidth = width;
  bmi_.bmiHeader.biHeight = -height;
  I_LOG("3");
  bmi_.bmiHeader.biSizeImage =
    width * height * (bmi_.bmiHeader.biBitCount >> 3);
  /*向VideoTrack订阅视频帧*/
  if (!rendered_track_)
    I_LOG("nullptr");
  else {
    I_LOG("!nullptr");
  }
  rendered_track_->AddOrUpdateSink(this, rtc::VideoSinkWants());
}

VideoRenderer::~VideoRenderer() {
  rendered_track_->RemoveSink(this);
  ::DeleteCriticalSection(&buffer_lock_);
}

void VideoRenderer::SetSize(int width, int height) {
  AutoLock<VideoRenderer> lock(this);

  if (width == bmi_.bmiHeader.biWidth && height == bmi_.bmiHeader.biHeight) {
    return;
  }

  bmi_.bmiHeader.biWidth = width;
  bmi_.bmiHeader.biHeight = -height;
  bmi_.bmiHeader.biSizeImage =
    width * height * (bmi_.bmiHeader.biBitCount >> 3);
  image_.reset(new uint8_t[bmi_.bmiHeader.biSizeImage]);
}

void VideoRenderer::OnFrame(const webrtc::VideoFrame& video_frame) {
  {
    AutoLock<VideoRenderer> lock(this);
    //I_LOG("on frame");
    /*将获取的视频帧，转成YUV420格式*/
    
    rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(
      video_frame.video_frame_buffer()->ToI420());
    
    
    if (video_frame.rotation() != webrtc::kVideoRotation_0) {
      buffer = webrtc::I420Buffer::Rotate(*buffer, video_frame.rotation());
    }

    SetSize(buffer->width() / 2, buffer->height() / 2);
    //I_LOG("bmi_.bmiHeader.biSizeImage：{}  ", bmi_.bmiHeader.biSizeImage);
    uint8_t* ydata = new uint8_t[bmi_.bmiHeader.biSizeImage];
    uint8_t* udata = new uint8_t[bmi_.bmiHeader.biSizeImage];
    uint8_t* vdata = new uint8_t[bmi_.bmiHeader.biSizeImage];

    //uint8_t* src = new uint8_t[bmi_.bmiHeader.biSizeImage];

    RTC_DCHECK(image_.get() != NULL);

    int stride_y = buffer->width() / 2;
    int stride_u = (buffer->width() / 2 + 1) / 2;
    int stride_v = (buffer->width() / 2 + 1) / 2;

    //缩放
    libyuv::Scale(buffer->DataY(), buffer->DataU(), buffer->DataV(), buffer->StrideY(), buffer->StrideU(), buffer->StrideV(),
      buffer->width(), buffer->height(),
      ydata, udata, vdata, stride_y, stride_u, stride_v,
      buffer->width() / 2, buffer->height() / 2, libyuv::kFilterBilinear);

    /*将YUV420格式的图像，转成RGB格式的图像。*/
    libyuv::I420ToABGR(ydata, stride_y, udata,
      stride_u, vdata, stride_v,
      //src,
      image_.get(),
      bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
      buffer->width() / 2, buffer->height() / 2);

    D_LOG("buffer w = {}, h = {}", buffer->width(), buffer->height());
    /*将图像镜像反转*/
    /*libyuv::ARGBMirror(src, bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
      image_.get(), bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
      buffer->width(), buffer->height());
    
    delete src;
    */
    delete ydata;
    delete udata;
    delete vdata;
  }

  /*触发渲染*/
  paint();
}