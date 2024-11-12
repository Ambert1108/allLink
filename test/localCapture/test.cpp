#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "pc/video_track_source.h"
#include "test/vcm_capturer.h"
#include "api/video/i420_buffer.h"
#include "third_party/libyuv/include/libyuv/convert_argb.h"
#include "libyuv.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "savequeue.h"
#include "seeker/logger.h"
#include "seeker/loggerApi.h"

#include <atomic>

struct ImageData {
  BITMAPINFO bmi;
  std::unique_ptr<uint8_t[]> image = nullptr;

  ImageData() = default;

  ImageData(const BITMAPINFO& bm, const uint8_t* data) : bmi(bm) {
    image.reset(new uint8_t[bmi.bmiHeader.biSizeImage]);
    memcpy(image.get(), data, bmi.bmiHeader.biSizeImage);
  }

  ImageData(const ImageData& other) : bmi(other.bmi) {
    if (other.image) {
      image.reset(new uint8_t[bmi.bmiHeader.biSizeImage]);
      std::copy(other.image.get(), other.image.get() + bmi.bmiHeader.biSizeImage, image.get());
    }
  }

  ImageData& operator=(const ImageData& other) {
    if (this != &other) {
      image.reset();
      bmi = other.bmi;

      if (other.image) {
        image.reset(new uint8_t[bmi.bmiHeader.biSizeImage]);
        std::copy(other.image.get(), other.image.get() + bmi.bmiHeader.biSizeImage, image.get());
      }
    }
    return *this;
  }
};

template <typename T>
class AutoLock {
public:
  explicit AutoLock(T* obj) : obj_(obj) { obj_->Lock(); }
  ~AutoLock() { obj_->Unlock(); }

protected:
  T* obj_;
};

class VideoRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
  VideoRenderer(std::function<void()> callback,
    int width,
    int height) : paint(callback) {
    ::InitializeCriticalSection(&buffer_lock_);
    ZeroMemory(&bmi_, sizeof(bmi_));
    bmi_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi_.bmiHeader.biPlanes = 1;
    bmi_.bmiHeader.biBitCount = 32;
    bmi_.bmiHeader.biCompression = BI_RGB;
    bmi_.bmiHeader.biWidth = width;
    bmi_.bmiHeader.biHeight = -height;
    bmi_.bmiHeader.biSizeImage =
      width * height * (bmi_.bmiHeader.biBitCount >> 3);
    /*向VideoTrack订阅视频帧*/
    //rendered_track_->AddOrUpdateSink(this, rtc::VideoSinkWants());
  }
  virtual ~VideoRenderer() {
    //rendered_track_->RemoveSink(this);
    ::DeleteCriticalSection(&buffer_lock_);
  }

  void Lock() { ::EnterCriticalSection(&buffer_lock_); }

  void Unlock() { ::LeaveCriticalSection(&buffer_lock_); }

  // VideoSinkInterface implementation
  void OnFrame(const webrtc::VideoFrame& frame) override {
    {
      AutoLock<VideoRenderer> lock(this);

      /*将获取的视频帧，转成YUV420格式*/
      rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(
        frame.video_frame_buffer()->ToI420());
      if (frame.rotation() != webrtc::kVideoRotation_0) {
        buffer = webrtc::I420Buffer::Rotate(*buffer, frame.rotation());
      }

      SetSize(buffer->width(), buffer->height());

      uint8_t* src = new uint8_t[bmi_.bmiHeader.biSizeImage];

      RTC_DCHECK(image_.get() != NULL);
      /*将YUV420格式的图像，转成RGB格式的图像。*/
      libyuv::I420ToABGR(buffer->DataY(), buffer->StrideY(), buffer->DataU(),
        buffer->StrideU(), buffer->DataV(), buffer->StrideV(),
        src,
        bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
        buffer->width(), buffer->height());

      /*将图像镜像反转*/
      libyuv::ARGBMirror(src, bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
        image_.get(), bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
        buffer->width(), buffer->height());
      delete src;
    }

    /*触发渲染*/
    paint();
  }

  const BITMAPINFO& bmi() const { return bmi_; }
  const uint8_t* image() const { return image_.get(); }

protected:
  void SetSize(int width, int height) {
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

  enum {
    SET_SIZE,
    RENDER_FRAME,
  };

  std::function<void()> paint = nullptr;
  BITMAPINFO bmi_;
  std::unique_ptr<uint8_t[]> image_;
  CRITICAL_SECTION buffer_lock_;
  //rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
};

std::atomic<bool> isMirror{ false };
std::unique_ptr<VideoRenderer> local_renderer_ = nullptr;
base::ThreadSafeQueue<ImageData> localImageList{};

void OnPaint() {
  //获取本地和远端的视频画面
  VideoRenderer* local_renderer = local_renderer_.get();
  if (local_renderer) {
    AutoLock<VideoRenderer> local_lock(local_renderer);
    const BITMAPINFO& lbmi = local_renderer->bmi();
    const uint8_t* limage = local_renderer->image();
    if (isMirror.load()) {
      ImageData data;
      data.bmi = lbmi;
      data.image.reset(new uint8_t[lbmi.bmiHeader.biSizeImage]);
      libyuv::ARGBMirror(limage, lbmi.bmiHeader.biWidth * lbmi.bmiHeader.biBitCount / 8,
        data.image.get(), lbmi.bmiHeader.biWidth * lbmi.bmiHeader.biBitCount / 8,
        lbmi.bmiHeader.biWidth, std::abs(lbmi.bmiHeader.biHeight));
      localImageList.Push(data);
    }
    else localImageList.Push(ImageData(lbmi, limage));
  }
  else {
    //TODO：不处于渲染状态，显示黑屏
  }
}

int main() {
	sf::RenderWindow* wnd = new sf::RenderWindow(
    sf::VideoMode(sf::VideoMode::getDesktopMode().width / 2,
      sf::VideoMode::getDesktopMode().height / 2),
    "local Capture",
    sf::Style::Close);
  wnd->setFramerateLimit(60);
  int wndWidth = wnd->getSize().x;
  int wndHeight = wnd->getSize().y;
  sf::Texture* localSource = new sf::Texture();
  sf::Sprite localVideo;
  sf::Event event{};

  webrtc::VideoCaptureModule::DeviceInfo* deviceInfo =
    webrtc::VideoCaptureFactory::CreateDeviceInfo();
  uint32_t number = deviceInfo->NumberOfDevices();
  std::cout << "video capture number " << number << std::endl;
  char deviceName[128] = { 0 };
  char deviceUniqueId[128] = { 0 };
  deviceInfo->GetDeviceName(0, deviceName, 128, deviceUniqueId, 128);
  std::cout << "device name: " << deviceName << "\ndevice unique id: " << deviceUniqueId << std::endl;
  webrtc::VideoCaptureCapability cap;
  int32_t capNum = deviceInfo->NumberOfCapabilities(deviceUniqueId);
  std::cout << "cap num " << capNum << std::endl;
  webrtc::VideoCaptureCapability requestCap;
  requestCap.width = 640;
  requestCap.height = 480;
  requestCap.maxFPS = 30;
  requestCap.videoType = webrtc::VideoType::kI420;
  webrtc::VideoCaptureCapability bestCap;
  int32_t num = deviceInfo->GetBestMatchedCapability(deviceUniqueId, requestCap, bestCap);
  std::cout << "best match num " << num << std::endl;
  local_renderer_.reset(new VideoRenderer(std::bind(OnPaint), 1, 1));
  rtc::scoped_refptr<webrtc::VideoCaptureModule> pVCM 
    = webrtc::VideoCaptureFactory::Create(deviceUniqueId);
  pVCM->RegisterCaptureDataCallback(local_renderer_.get());
  pVCM->StartCapture(bestCap);

  while (wnd->isOpen()) {
    while (wnd->pollEvent(event)) {
      switch (event.type) {
      case sf::Event::Closed:
        wnd->close();
        break;

      case sf::Event::KeyPressed:
        if (event.key.code == sf::Keyboard::Escape) {
          wnd->close();
        }
        else if (event.key.code == sf::Keyboard::M) {
          isMirror.store(!isMirror);
        }
        break;
      }
    }
    
    wnd->clear();
    ImageData localData;
    if (localImageList.TryPopFlex(localData)) {
      int localHeight = abs(localData.bmi.bmiHeader.biHeight);
      int localWidth = localData.bmi.bmiHeader.biWidth;
      bool reset = false;
      if (localSource->getSize().x != localWidth
        || localSource->getSize().y != localHeight) {
        D_LOG("remote size is {}:{}, raw size is {}:{}", localSource->getSize().x, localSource->getSize().y,
          remoteWidth, remoteHeight);
        localSource->create(localWidth, localHeight);
        reset = true;
      }
      localSource->update(localData.image.get());
      localVideo.setTexture(*localSource, reset);
      int localX = (wndWidth - localWidth) / 2;
      int localY = (wndHeight - localHeight) / 2;
      localVideo.setPosition(sf::Vector2f(localX, localY));
      wnd->draw(localVideo);
    }
    wnd->display();
  }
	return 0;
}