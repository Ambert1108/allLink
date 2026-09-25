#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "pc/video_track_source.h"
#include "test/vcm_capturer.h"
#include "api/video/i420_buffer.h"
#include "third_party/libyuv/include/libyuv/convert_argb.h"
#include "libyuv.h"


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

class VideoRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
  VideoRenderer(std::function<void()> callback,
    int width,
    int height,
    webrtc::VideoTrackInterface* track_to_render);
  virtual ~VideoRenderer();

  void Lock() { ::EnterCriticalSection(&buffer_lock_); }

  void Unlock() { ::LeaveCriticalSection(&buffer_lock_); }

  // VideoSinkInterface implementation
  void OnFrame(const webrtc::VideoFrame& frame) override;

  const BITMAPINFO& bmi() const { return bmi_; }
  const uint8_t* image() const { return image_.get(); }

protected:
  void SetSize(int width, int height);

  enum {
    SET_SIZE,
    RENDER_FRAME,
  };

  std::function<void()> paint = nullptr;
  BITMAPINFO bmi_;
  std::unique_ptr<uint8_t[]> image_;
  CRITICAL_SECTION buffer_lock_;
  rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
};

template <typename T>
class AutoLock {
public:
  explicit AutoLock(T* obj) : obj_(obj) { obj_->Lock(); }
  ~AutoLock() { obj_->Unlock(); }

protected:
  T* obj_;
};