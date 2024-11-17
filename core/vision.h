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
#include "signling.h"
#include "message.h"
#include "screen/screensink.h"

namespace alllink {
  class VisionCnetralCallback {
  public:
    /*通知控制器登录信令服务器*/
    virtual bool StartLogin(const LinkInfo& link, const UserInfo& user) = 0;
    /*通知控制器登出信令服务器*/
    virtual void DisconnectFromServer() = 0;
    /*通知控制器连接对端peer*/
    virtual bool ConnectToPeer(int peer_id) = 0;
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

    virtual void registerObserver(VisionCnetralCallback* callback) = 0;

    virtual void switchNextScreen() = 0;

    virtual void switchLastScreen() = 0;

    virtual void startLocalRenderer(webrtc::VideoTrackInterface* local_video) = 0;
    virtual void stopLocalRenderer() = 0;
    virtual void startRemoteRenderer(webrtc::VideoTrackInterface* remote_video) = 0;
    virtual void stopRemoteRenderer() = 0;

    virtual void sendCustomMessage(int msg_id, void* data) = 0;
  };

  class VisionCentralContoller : public VisionCnetralBase {
  public:

    VisionCentralContoller();
    ~VisionCentralContoller();

    void registerObserver(VisionCnetralCallback* callback);
    void run();

    void switchNextScreen();

    void switchLastScreen();

    void startLocalRenderer(webrtc::VideoTrackInterface* local_video);
    void stopLocalRenderer();
    void startRemoteRenderer(webrtc::VideoTrackInterface* remote_video);
    void stopRemoteRenderer();

    void sendCustomMessage(int msg_id, void* data);

  protected:
    void pollEvent();
    void update();
    void render();

  private:
    std::shared_ptr<BaseScreen> wnd = nullptr; //流式窗口
    std::unique_ptr<BaseScreen> loginWnd = nullptr;
    VisionCnetralCallback* callback_;
    Message msg;
  };
}
