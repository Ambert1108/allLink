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
#include "seeker/iniConfig.hpp"

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

  struct SignlingInteractionObserver {
    /*通知 中控器 接收到信令转发的其他客户端(p2p流程)发送过来的信息*/
    virtual void OnMessageFromSignling(const SignInfo& info) = 0;

    /* 通知 中控器 其他客户端挂断通话 */
    virtual void OnPeerDisconnected(const std::string& id) = 0;

    virtual void OnSignlingDisconnect() = 0;

  protected:
    virtual ~SignlingInteractionObserver() {}
  };

  class SignlingInteractionSystem : public WSListenObserver {
  public:
    enum State {
      NONE,

      // ws已连接
      CONNECT_ON = 1,

      // 已登录信令服务器
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

    bool connectServer(const ServerInfo& info);

    bool login(const UserInfo& info);

    void disConnectServer();

    bool reLogin();

    bool sendToPeer(const std::string& to, const std::string& message);

    void sendTrickle(const std::string& to, const Candidate& ice);

    void sendTrickleComplete(const std::string& to);

    bool sendAck(const std::string& to);

    bool sendInfo(const std::string& to, int info);

    bool sendBye(const std::string& to);

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
    bool connect(const ServerInfo&);
    bool ToSignaling(const SignInfo& info);

    SignlingInteractionObserver* callback_;
    static constexpr const char* TAG = "WSClient";
    mutable std::mutex Locker{};
    std::shared_ptr<oatpp::websocket::WebSocket> client;
    std::shared_ptr<WSListener> listener;
    aom::InvokeTimerPtr listenBody;
    aom::InvokeTimerPtr keepBody;
    ServerInfo serverInfo;
    UserInfo userInfo;
    State signalState{ NONE };
    int64_t lastBeatPoint = 0;
    int64_t cseq_ = 0;
    std::string callId;
    int mode = 0; //1v1通话:0, 会议流程:1
  };
}