#pragma once

#include <map>
#include <memory>
#include <string>
#include <mutex>

#include "api/async_dns_resolver.h"
#include "api/task_queue/pending_task_safety_flag.h"
#include "rtc_base/net_helpers.h"
#include "rtc_base/physical_socket_server.h"

#include "seeker/logger.h"
#include "seeker/loggerApi.h"

#include "wslistener.h"

namespace alllink {

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
      NOT_CONNECTED,
      RESOLVING,
      SIGNING_IN,
      CONNECTED,
      SIGNING_OUT_WAITING,
      SIGNING_OUT,
    };

    SignlingInteractionSystem();
    ~SignlingInteractionSystem();

    bool isConnected() const;

    void registerObserver(SignlingInteractionObserver* callback);

    void connect(const std::string& server, int port);

  protected:
    //
    // WSListenObserver implementation.
    //

    void OnINVITE(const SignInfo& info) override;
    void OnOK(const SignInfo& info) override;
    void OnBYE(const SignInfo& info) override;
    void OnCANCEL(const SignInfo& info) override;
    void OnACK(const SignInfo& info) override;
    void OnUnauthorized(const SignInfo& info) override;
    void OnHeartbeat(const SignInfo& info) override;

  private:
    SignlingInteractionObserver* callback_;
    static constexpr const char* TAG = "WSClient";
    mutable std::mutex locker_{};
  };
}