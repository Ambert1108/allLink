#pragma once
#include "oatpp-websocket/Connector.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"
#include "oatpp-websocket/ConnectionHandler.hpp"
#include "oatpp-websocket/WebSocket.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "signinfo.hpp"

#include "seeker/logger.h"
#include "seeker/loggerApi.h"

namespace alllink {
  class JanusListenObserver {
  public:
    virtual void OnSuccess(const JanusReponse& resp) = 0;
    virtual void OnAck(const JanusReponse& resp) = 0;
    virtual void OnEvent(const JanusReponse& resp) = 0;
  protected:
    virtual ~JanusListenObserver() {}
  };

  class JanusListener : public oatpp::websocket::WebSocket::Listener {
  public:
    JanusListener();

    void registerObserver(JanusListenObserver* callback);

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

  private:
    static constexpr const char* TAG = "Client_WSListener";
    oatpp::data::stream::BufferOutputStream messageBuffer;
    JanusListenObserver* callback_;
  };
}