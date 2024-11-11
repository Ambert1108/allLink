#pragma once

#include <map>
#include <memory>
#include <string>
#include <mutex>
#include <atomic>

#include "api/media_stream_interface.h"
#include "api/video/video_frame.h"
#include "media/base/media_channel.h"
#include "media/base/video_common.h"
#if defined(WEBRTC_WIN)
#include "rtc_base/win32.h"
#endif  // WEBRTC_WIN
#include "mediaclient.h"
#include "message.h"

#include <SFML/Graphics.hpp>

namespace alllink {
  class VisionCnetralCallback {
  public:
    /*通知控制器登录信令服务器*/
    virtual void StartLogin(const std::string& server, int port) = 0;
    /*通知控制器登出信令服务器*/
    virtual void DisconnectFromServer() = 0;
    /*通知控制器连接对端peer*/
    virtual void ConnectToPeer(int peer_id) = 0;
    /*通知控制器与对端断开连接*/
    virtual void DisconnectFromCurrentPeer() = 0;
    /*控制器自定义消息处理函数*/
    virtual void CustomMessageCallback(int msg_id, void* data) = 0;
    /*通知控制器关闭*/
    virtual void Close() = 0;

  protected:
    virtual ~VisionCnetralCallback() {}
  };

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

  class VisionCnetralBase {
  public:
    virtual ~VisionCnetralBase() {}

    enum UI {
      LOGINING,
      MEETING,
      STREAMING,
    };

    virtual void RegisterObserver(VisionCnetralCallback* callback) = 0;

    virtual bool IsWindow() = 0;

    virtual UI current_ui() = 0;

    virtual void switchToLoginingUI() = 0;
    virtual void switchToMeetingUI() = 0;
    virtual void switchToStreamingUI() = 0;

    virtual void startLocalRenderer(webrtc::VideoTrackInterface* local_video) = 0;
    virtual void stopLocalRenderer() = 0;
    virtual void startRemoteRenderer(
      webrtc::VideoTrackInterface* remote_video) = 0;
    virtual void stopRemoteRenderer() = 0;

    virtual void sendCustomMessage(int msg_id, void* data) = 0;
  };

  class VisionCentralContoller {
  public:

    VisionCentralContoller(const char* server, int port);
    ~VisionCentralContoller();

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

    // A little helper class to make sure we always to proper locking and
    // unlocking when working with VideoRenderer buffers.
    template <typename T>
    class AutoLock {
    public:
      explicit AutoLock(T* obj) : obj_(obj) { obj_->Lock(); }
      ~AutoLock() { obj_->Unlock(); }

    protected:
      T* obj_;
    };

  protected:

    void createWindow();
    void render();

  private:
    std::unique_ptr<VideoRenderer> local_renderer_;
    std::unique_ptr<VideoRenderer> remote_renderer_;
    sf::RenderWindow* wnd;
    sf::Event event{};
    sf::Texture* localSrc = nullptr;
    sf::Texture* remoteSrc = nullptr;
    sf::Sprite localVideo{};
    sf::Sprite remoteVideo{};
    VisionCnetralCallback* callback_;
    std::string server_;
    std::string port_;
    base::ThreadSafeQueue<ImageData> remoteImageList{};
    base::ThreadSafeQueue<ImageData> localImageList{};
    int wndWidth = 0;
    int wndHeight = 0;
    std::atomic<bool> isMirror{ false };
  };
}
