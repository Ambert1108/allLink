#pragma once

#include <map>
#include <memory>
#include <string>
#include <mutex>
#include <regex>

#include "api/async_dns_resolver.h"
#include "api/task_queue/pending_task_safety_flag.h"
#include "rtc_base/net_helpers.h"
#include "rtc_base/physical_socket_server.h"

#include "seeker/logger.h"
#include "seeker/loggerApi.h"

#include "wslistener.h"
#include "message.h"
#include "utils/InvokeTimer.hpp"

namespace alllink {
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
  };

  struct LinkInfo {
    std::string serverIp_;
    uint16_t serverPort_;

    LinkInfo() = default;

    explicit LinkInfo(const std::string& addr) {
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
        E_LOG("[LinkInfo::conductor] Failed to resolve server addr:{}", addr);
      }
    }

    bool operator==(const LinkInfo& other) const {
      return serverIp_ == other.serverIp_ && serverPort_ == other.serverPort_;
    }

    LinkInfo& operator=(const LinkInfo& other) {
      if (this != &other) {
        serverIp_ = other.serverIp_;
        serverPort_ = other.serverPort_;
      }
      return *this;
    }
  };

  struct SignlingInteractionObserver {
    /*通知 控制器 信令服务器成功登录*/
    virtual void OnSignedIn() = 0;
    /*通知 控制器 登出信令服务器*/
    virtual void OnDisconnected() = 0;
    /*通知 控制器 其他客户端登录信令服务器*/
    virtual void OnPeerConnected(int id, const std::string& name) = 0;
    /*通知 控制器 其他客户端登出信令服务器*/
    virtual void OnPeerDisconnected(int peer_id) = 0;
    /*通知 控制器 接收到其他客户端发送过来的信息*/
    virtual void OnMessageFromPeer(int peer_id, const std::string& message) = 0;
    /*通知 控制器 信息已发送*/
    virtual void OnMessageSent(int err) = 0;
    /*通知 控制器 登录信令服务器失败*/
    virtual void OnServerConnectionFailure() = 0;

  protected:
    virtual ~SignlingInteractionObserver() {}
  };

  class SignlingInteractionSystem : public WSListenObserver {
  public:
    enum State {
      NONE,

      // 未登录
      LOGIN_OUT = 1,

      // 已登录
      LOGIN_ON,

      // 作为主叫生成Offer SDP并发送FORWARD后，等待接收Trying
      CALLING,

      // 作为主叫收到Trying后，继续等待接收Ringing
      TRYING,

      // 作为主叫收到Ringing后，继续等待接收OK，并从中取出SDP
      RINGING,

      // 作为主叫收到OK并成功解析SDP后，作为主叫接通
      CALLER,

      // 作为被叫收到FORWARD后，等待用户响应以发送Ringing
      FORWARDING,

      // 作为被叫用户确认接通后，生成Answer SDP并发送OK
      RINGEE,

      // 作为被叫发送OK后，等待收到ACK
      ACKING,

      // 作为被叫收到ACK后，作为被叫接通
      CALLEE
    };

    SignlingInteractionSystem();
    ~SignlingInteractionSystem();

    bool isConnected() const;

    void registerObserver(SignlingInteractionObserver* callback);

    bool connectServer(const LinkInfo& info);

    bool login(const UserInfo& info);

  protected:
    void logout();
    //
    // WSListenObserver implementation.
    //

    void OnFORWARD(const SignInfo& info) override;
    void OnACK(const SignInfo& info) override;
    void OnBYE(const SignInfo& info) override;
    void OnCANCEL(const SignInfo& info) override;
    void OnHeartbeat(const SignInfo& info) override;

    void OnOK(const SignInfo& info) override;
    void OnTrying(const SignInfo & info) override;
    void OnRinging(const SignInfo& info) override;
    void OnUnauthorized(const SignInfo& info) override;

  private:
    bool ToREGISTER(const SignInfo& info);
    bool ToFORWARD(const SignInfo& info);
    bool ToACK(const SignInfo& info);
    bool ToBYE(const SignInfo& info);
    bool ToCANCEL(const SignInfo& info);
    bool ToINFO(const SignInfo& info);
    bool ToHeartbeat(const SignInfo& info);
    bool ToOK(const SignInfo& info);
    bool ToTrying(const SignInfo& info);
    bool ToRinging(const SignInfo& info);

    SignlingInteractionObserver* callback_;
    static constexpr const char* TAG = "WSClient";
    std::shared_ptr<oatpp::websocket::WebSocket> client;
    std::shared_ptr<WSListener> listener;
    aom::InvokeTimerPtr listenBody;
    LinkInfo linkInfo;
    UserInfo userInfo;
    State signalState{ LOGIN_OUT };
    int64_t lastBeatPoint = 0;
    int64_t HeartbeatInterval = 1000; //心跳间隔
    int64_t cseq_ = 0;
  };
}