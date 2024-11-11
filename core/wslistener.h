#pragma once
#include "oatpp-websocket/ConnectionHandler.hpp"
#include "oatpp-websocket/WebSocket.hpp"

#include "signinfo.h"

namespace alllink {
  class WSListenObserver {
    virtual void OnINVITE(const SignInfo& info) = 0;
    virtual void OnOK(const SignInfo& info) = 0;
    virtual void OnBYE(const SignInfo& info) = 0;
    virtual void OnCANCEL(const SignInfo& info) = 0;
    virtual void OnACK(const SignInfo& info) = 0;
    virtual void OnUnauthorized(const SignInfo& info) = 0;
    virtual void OnHeartbeat(const SignInfo& info) = 0;
  protected:
    virtual ~WSListenObserver() {}
  };

  class WSListener : public oatpp::websocket::WebSocket::Listener {
  public:
    WSListener(std::mutex& lock);

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

  private:
    static constexpr const char* TAG = "Client_WSListener";
    std::mutex& locker_;
    oatpp::data::stream::BufferOutputStream messageBuffer;
    WSListenObserver* callback_;
  };
}