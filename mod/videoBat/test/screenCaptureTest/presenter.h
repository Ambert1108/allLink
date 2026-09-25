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

#include <SFML/Graphics.hpp>

namespace wt{
  class MainWndCallback {
  public:
    /*通知登录信令服务器*/
    virtual void StartLogin(const std::string& server, int port) = 0;
    /*通知登出信令服务器*/
    virtual void DisconnectFromServer() = 0;
    /*连接对端peer*/
    virtual void ConnectToPeer(int peer_id) = 0;
    /*与对端断开连接*/
    virtual void DisconnectFromCurrentPeer() = 0;
    /*自定义消息处理函数*/
    virtual void UIThreadCallback(int msg_id, void* data) = 0;
    /*关闭Conductor*/
    virtual void Close() = 0;

  protected:
    virtual ~MainWndCallback() {}
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

  // Pure virtual interface for the main window.
  class MainWindow {
  public:
    virtual ~MainWindow() {}

    enum UI {
      CONNECT_TO_SERVER,
      LIST_PEERS,
      STREAMING,
    };

    virtual void RegisterObserver(MainWndCallback* callback) = 0;

    virtual bool IsWindow() = 0;
    //virtual void messageBox(const char* caption,
    //  const char* text,
    //  bool is_error) = 0;

    virtual UI current_ui() = 0;

    virtual void SwitchToConnectUI() = 0;
    virtual void SwitchToPeerList(const Peers& peers) = 0;
    virtual void SwitchToStreamingUI(bool flag = false) = 0;

    virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video) = 0;
    virtual void StopLocalRenderer() = 0;
    virtual void StartRemoteRenderer(
      webrtc::VideoTrackInterface* remote_video) = 0;
    virtual void StopRemoteRenderer() = 0;

    virtual void QueueUIThreadCallback(int msg_id, void* data) = 0;
  };

  class MainWnd : public MainWindow {
  public:
    static const wchar_t kClassName[];

    enum WindowMessages {
      UI_THREAD_CALLBACK = WM_APP + 1,
    };

    MainWnd(const char* server, int port);
    ~MainWnd();

    bool Create();
    bool Destroy();
    bool PreTranslateMessage();
    bool PreTranslateMessage(MSG* msg);
    bool PreTranslateMessage(Message msg);
    bool isExist(const std::string& msg);

    virtual void RegisterObserver(MainWndCallback* callback);
    virtual bool IsWindow();
    virtual void SwitchToConnectUI();
    virtual void SwitchToPeerList(const Peers& peers);
    virtual void SwitchToStreamingUI(bool isRecv);
    virtual UI current_ui() { return ui_; }

    virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video);
    virtual void StopLocalRenderer();
    virtual void StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video);
    virtual void StopRemoteRenderer();

    virtual void QueueUIThreadCallback(int msg_id, void* data);

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

    void OnPaint();
    void OnDestroyed();

    void OnDefaultAction();

    void createWindow();
    void render();

  private:
    std::unique_ptr<VideoRenderer> local_renderer_;
    std::unique_ptr<VideoRenderer> remote_renderer_;
    UI ui_;
    sf::RenderWindow* wnd;
    sf::Event event{};
    sf::Texture* localSrc = nullptr;
    sf::Texture* remoteSrc = nullptr;
    sf::Sprite localVideo{};
    sf::Sprite remoteVideo{};
    mutable std::mutex wndLocker{};
    MainWndCallback* callback_;
    std::string server_;
    std::string port_;
    zx::ThreadSafeQueue<ImageData> remoteImageList{};
    zx::ThreadSafeQueue<ImageData> localImageList{};
    int wndWidth = 0;
    int wndHeight = 0;
    bool wndVisible = false;
    std::atomic<bool> isMirror{ false };
  };
}
