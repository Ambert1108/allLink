#include "januslistener.h"
#include "seeker/json.hpp"

namespace alllink {
  JanusListener::JanusListener() {}

  void JanusListener::registerObserver(JanusListenObserver* callback) { callback_ = callback; }

  void JanusListener::onPing(const WebSocket& socket, const oatpp::String& message) {
    OATPP_LOGD(TAG, "onPing");
    //socket.sendPong(message);
  }

  void JanusListener::onPong(const WebSocket& socket, const oatpp::String& message) {
    OATPP_LOGD(TAG, "onPong");
  }

  void JanusListener::onClose(const WebSocket& socket, v_uint16 code, const oatpp::String& message) {
    OATPP_LOGD(TAG, "onClose code=%d", code);
    //socket.sendClose();
  }

  void JanusListener::readMessage(const WebSocket& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) {

    if (size == 0) { // message transfer finished

      try {
        auto wholeMessage = messageBuffer.toString();
        messageBuffer.setCurrentPosition(0);
        // 根据消息类型调用回调
        JanusReponse resp;
        seeker::json::fromJsonString(resp, wholeMessage->c_str());
        I_LOG("janus on message received {}", *wholeMessage.get());
        if (resp.janus == "success") callback_->OnSuccess(resp);
        else if (resp.janus == "ack") callback_->OnAck(resp);
        else if (resp.janus == "event") callback_->OnEvent(resp);
        else W_LOG("[JanusListener::readMessage] get unknown message, type={}", resp.janus);
      }
      catch (std::exception& ex) {
        E_LOG("janus listener catch exception:{}", ex.what());
        return;
      }
      catch (...) {
        E_LOG("janus listener catch unknown exception");
        return;
      }
    }
    else if (size > 0) { // message frame received
      messageBuffer.writeSimple(data, size);
    }

  }
}