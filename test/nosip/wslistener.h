#pragma once
#include "oatpp-websocket/Connector.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"
#include "oatpp-websocket/ConnectionHandler.hpp"
#include "oatpp-websocket/WebSocket.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "signinfo.hpp"

#include "seeker/logger.h"
#include "seeker/loggerApi.h"

#include <queue>

namespace alllink {
  class WSListenObserver {
  public:
    /* 一般请求 */

    virtual void OnFORWARD(const SignInfo& info) = 0;
    virtual void OnACK(const SignInfo& info) = 0;
    virtual void OnBYE(const SignInfo& info) = 0;
    virtual void OnCANCEL(const SignInfo& info) = 0;
    virtual void OnHeartbeat(const SignInfo& info) = 0;

    /* 一般响应 */

    virtual void OnOK(const SignInfo& info) = 0;
    virtual void OnTrying(const SignInfo& info) = 0;
    virtual void OnRinging(const SignInfo& info) = 0;
    virtual void OnUnauthorized(const SignInfo& info) = 0;
  protected:
    virtual ~WSListenObserver() {}
  };

  class WSListener : public oatpp::websocket::WebSocket::Listener {
  public:
    WSListener();

    void registerObserver(WSListenObserver* callback);

    /**
     * Called on "ping" frame.
     */
    void onPing(const WebSocket& socket, const oatpp::String& message) override;

    /**
     * Called on "pong" frame
     */
    void onPong(const WebSocket& socket, const oatpp::String& message) override;

    /**
     * Called on "close" frame
     */
    void onClose(const WebSocket& socket, v_uint16 code, const oatpp::String& message) override;

    /**
     * Called on each message frame. After the last message will be called once-again with size == 0 to designate end of the message.
     */
    void readMessage(const WebSocket& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;

    int getMsg(oatpp::String&);

  private:
    static constexpr const char* TAG = "Client_WSListener";
    oatpp::data::stream::BufferOutputStream messageBuffer;
    WSListenObserver* callback_;
    std::mutex Locker{};
    std::queue<oatpp::String> msgList{};
  };
}