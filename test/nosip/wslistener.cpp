#include "wslistener.h"

namespace alllink {
  WSListener::WSListener() {}

  void WSListener::registerObserver(WSListenObserver* callback) { callback_ = callback; }

  void WSListener::onPing(const WebSocket& socket, const oatpp::String& message) {
    OATPP_LOGD(TAG, "onPing");
    //socket.sendPong(message);
  }

  void WSListener::onPong(const WebSocket& socket, const oatpp::String& message) {
    OATPP_LOGD(TAG, "onPong");
  }

  void WSListener::onClose(const WebSocket& socket, v_uint16 code, const oatpp::String& message) {
    OATPP_LOGD(TAG, "onClose code=%d", code);
    socket.sendClose();
  }

  void WSListener::readMessage(const WebSocket& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) {

    if (size == 0) { // message transfer finished

      auto wholeMessage = messageBuffer.toString();
      messageBuffer.setCurrentPosition(0);
      //TODO:根据消息类型调用回调
      I_LOG("on message received {}", *wholeMessage.get());
    }
    else if (size > 0) { // message frame received
      messageBuffer.writeSimple(data, size);
    }

  }
}