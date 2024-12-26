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
    virtual bool StartLogin(const ServerInfo& server, const UserInfo& user) = 0;
    /*通知控制器登出信令服务器*/
    virtual void DisconnectFromServer() = 0;
    /*通知控制器连接对端peer*/
    virtual bool ConnectToPeer(const std::string& to) = 0;
    /*通知控制器与对端断开连接*/
    virtual void DisconnectFromCurrentPeer() = 0;
    /*控制器自定义消息处理函数*/
    virtual void CustomMessageCallback(const Message& msg) = 0;
    /*通知控制器关闭*/
    virtual void Close() = 0;

  protected:
    virtual ~VisionCnetralCallback() {}
  };

  class VisionCnetralBase {
  public:
    virtual ~VisionCnetralBase() {}

    virtual void registerObserver(VisionCnetralCallback* callback) = 0;

    virtual void startLocalRenderer(webrtc::VideoTrackInterface* local_video) = 0;
    virtual void stopLocalRenderer() = 0;
    virtual void startRemoteRenderer(webrtc::VideoTrackInterface* remote_video) = 0;
    virtual void stopRemoteRenderer() = 0;
  };

  class VisionCentralContoller : public VisionCnetralBase {
  public:
    enum class VisionType {
      LOGIN = 0,
      LOGOUT,
      RECONNECT
    };

    VisionCentralContoller();
    ~VisionCentralContoller();

    void registerObserver(VisionCnetralCallback* callback);
    void run();

    void startLocalRenderer(webrtc::VideoTrackInterface* local_video);
    void stopLocalRenderer();
    void startRemoteRenderer(webrtc::VideoTrackInterface* remote_video);
    void stopRemoteRenderer();

  protected:
    void pollEvent();
    void update();
    void render();

  private:
    std::shared_ptr<BaseScreen> wnd = nullptr; //流式窗口
    std::shared_ptr<BaseScreen> streamWnd = nullptr; //流式窗口
    std::shared_ptr<BaseScreen> loginWnd = nullptr;
    std::shared_ptr<BaseScreen> enterWnd = nullptr;
    VisionCnetralCallback* callback_;
    Message msg;
    VisionType type_{ VisionType::LOGOUT };
  };
}
