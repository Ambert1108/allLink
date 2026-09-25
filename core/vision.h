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
#include "message.h"
#include "screen/screensink.h"
#include "seeker/iniConfig.hpp"

#include "engine/signalBridge/rtcConnectEngine.h"

namespace linkinfo {
  struct UserInfo {
    std::string id_;
    std::string pwd_;

    UserInfo() = default;

    bool operator==(const UserInfo& other) const {
      return id_ == other.id_ && pwd_ == other.pwd_;
    }

    UserInfo& operator=(const UserInfo& other) {
      if (this != &other) {
        id_ = other.id_;
        pwd_ = other.pwd_;
      }
      return *this;
    }

    void clear() {
      id_.clear();
      pwd_.clear();
    }
  };

  struct ServerInfo {
    std::string serverIp_;
    uint16_t serverPort_;

    ServerInfo() = default;

    explicit ServerInfo(const std::string& addr) {
      try {
        std::regex pattern(R"((\d+\.\d+\.\d+\.\d+):(\d+))");
        std::smatch matches;
        if (!std::regex_match(addr, matches, pattern)) throw std::runtime_error("");
        if (matches.size() != 3) throw std::runtime_error("");
        // 0是整个匹配，1是IP，2是端口
        serverIp_ = matches[1];
        std::string port = matches[2];
        serverPort_ = std::stoi(port);
      }
      catch (std::exception& ex) {
        E_LOG("[ServerInfo::conductor] Failed to resolve server addr:{}", addr);
      }
    }

    bool operator==(const ServerInfo& other) const {
      return serverIp_ == other.serverIp_ && serverPort_ == other.serverPort_;
    }

    ServerInfo& operator=(const ServerInfo& other) {
      if (this != &other) {
        serverIp_ = other.serverIp_;
        serverPort_ = other.serverPort_;
      }
      return *this;
    }

    void clear() {
      serverIp_.clear();
      serverPort_ = 0;
    }
  };
}


namespace alllink {
  class VisionCentralCallback {
  public:
    /*通知控制器登录信令服务器*/
    virtual bool LoginSignaling(const linkinfo::ServerInfo& server, const linkinfo::UserInfo& user) = 0;
    /*通知控制器登出信令服务器*/
    virtual void DisconnectFromServer() = 0;
    /*通知控制器创建会议*/
    virtual bool CreateMeeting(const std::wstring& videoEnc, const std::wstring& audioEnc, const std::wstring& videoMcu, const std::wstring& audioMcu) = 0;
    /*通知控制器预定会议*/
    virtual void BookingMeeting(const std::wstring& videoEnc, const std::wstring& audioEnc, const std::wstring& videoMcu, const std::wstring& audioMcu) = 0;
    /*通知控制器加入会议*/
    virtual bool JoinMeeting(const std::string& to) = 0;
    /*通知控制器邀请用户入会*/
    virtual void InviteUser(const std::string& to) = 0;
    /*通知控制器与对端断开连接*/
    virtual void DisconnectFromCurrentPeer() = 0;
    /*控制器自定义消息处理函数*/
    virtual void CustomMessageCallback(const Message& msg) = 0;
    /*通知控制器关闭*/
    virtual void Close() = 0;

  protected:
    virtual ~VisionCentralCallback() {}
  };

  class VisionCentralBase {
  public:
    virtual ~VisionCentralBase() {}

    virtual void registerObserver(VisionCentralCallback* callback) = 0;

    virtual void startRemoteRenderer(webrtc::VideoTrackInterface* remote_video) = 0;
    virtual void stopRemoteRenderer() = 0;
  };

  class VisionCentralController : public VisionCentralBase {
  public:
    enum class VisionType {
      LOGIN = 0,
      LOGOUT,
      RECONNECT
    };

    VisionCentralController();
    ~VisionCentralController();

    void registerObserver(VisionCentralCallback* callback);
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
    std::shared_ptr<BaseScreen> inviteWnd = nullptr;
    VisionCentralCallback* callback_;
    Message msg;
    VisionType type_{ VisionType::LOGOUT };
  };
}
