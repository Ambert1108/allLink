#pragma once

#include <map>
#include <memory>
#include <string>

#include "api/async_dns_resolver.h"
#include "api/task_queue/pending_task_safety_flag.h"
#include "rtc_base/net_helpers.h"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/third_party/sigslot/sigslot.h"

#include "seeker/logger.h"
#include "seeker/loggerApi.h"

namespace wt {
  typedef std::map<int, std::string> Peers;

  struct MediaClientObserver {
    /*成功登录信令服务器*/
    virtual void OnSignedIn() = 0;
    /*登出信令服务器*/
    virtual void OnDisconnected() = 0;
    /*其他客户端登录信令服务器*/
    virtual void OnPeerConnected(int id, const std::string& name) = 0;
    /*其他客户端登出信令服务器*/
    virtual void OnPeerDisconnected(int peer_id) = 0;
    /*接收到其他客户端发送过来的信息*/
    virtual void OnMessageFromPeer(int peer_id, const std::string& message) = 0;
    /*通知信息已发送*/
    virtual void OnMessageSent(int err) = 0;
    /*登录信令服务器失败*/
    virtual void OnServerConnectionFailure() = 0;

  protected:
    virtual ~MediaClientObserver() {}
  };

  class MediaClient : public sigslot::has_slots<> {
  public:
    enum State {
      NOT_CONNECTED,
      RESOLVING,
      SIGNING_IN,
      CONNECTED,
      SIGNING_OUT_WAITING,
      SIGNING_OUT,
    };

    MediaClient();
    ~MediaClient();

    int id() const;
    bool is_connected() const;
    const Peers& peers() const;

    void RegisterObserver(MediaClientObserver* callback);

    void Connect(const std::string& server,
      int port,
      const std::string& client_name);

    bool SendToPeer(int peer_id, const std::string& message);
    bool SendHangUp(int peer_id);
    bool IsSendingMessage();

    bool SignOut();

  protected:
    void DoConnect();
    void Close();
    void InitSocketSignals();
    bool ConnectControlSocket();
    void OnConnect(rtc::Socket* socket);
    void OnHangingGetConnect(rtc::Socket* socket);
    void OnMessageFromPeer(int peer_id, const std::string& message);

    // Quick and dirty support for parsing HTTP header values.
    bool GetHeaderValue(const std::string& data,
      size_t eoh,
      const char* header_pattern,
      size_t* value);

    bool GetHeaderValue(const std::string& data,
      size_t eoh,
      const char* header_pattern,
      std::string* value);

    // Returns true if the whole response has been read.
    bool ReadIntoBuffer(rtc::Socket* socket,
      std::string* data,
      size_t* content_length);

    void OnRead(rtc::Socket* socket);

    void OnHangingGetRead(rtc::Socket* socket);

    // Parses a single line entry in the form "<name>,<id>,<connected>"
    bool ParseEntry(const std::string& entry,
      std::string* name,
      int* id,
      bool* connected);

    int GetResponseStatus(const std::string& response);

    bool ParseServerResponse(const std::string& response,
      size_t content_length,
      size_t* peer_id,
      size_t* eoh);

    void OnClose(rtc::Socket* socket, int err);

    void OnResolveResult(const webrtc::AsyncDnsResolverResult& result);

    MediaClientObserver* callback_;
    rtc::SocketAddress server_address_;
    std::unique_ptr<webrtc::AsyncDnsResolverInterface> resolver_;
    std::unique_ptr<rtc::Socket> control_socket_;
    std::unique_ptr<rtc::Socket> hanging_get_;
    std::string onconnect_data_;
    std::string control_data_;
    std::string notification_data_;
    std::string client_name_;
    Peers peers_;
    State state_;
    int my_id_;
    webrtc::ScopedTaskSafety safety_;
  };
}