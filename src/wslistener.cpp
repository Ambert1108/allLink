#include "wslistener.h"

namespace alllink {
  WSListener::WSListener() {}

  void WSListener::registerObserver(WSListenObserver* callback) { callback_ = callback; }

  void WSListener::onPing(const WebSocket& socket, const oatpp::String& message) {
     W_LOG("onPing");
    //socket.sendPong(message);
  }

  void WSListener::onPong(const WebSocket& socket, const oatpp::String& message) {
    W_LOG("onPong");
  }

  void WSListener::onClose(const WebSocket& socket, v_uint16 code, const oatpp::String& message) {
    W_LOG("onClose code={}", code);
    socket.sendClose();
  }

  void WSListener::readMessage(const WebSocket& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) {

    if (size == 0) { // message transfer finished
      try{
        auto wholeMessage = messageBuffer.toString(); 
        messageBuffer.setCurrentPosition(0);
        //TODO:根据消息类型调用回调
        I_LOG("signling on message received {}", *wholeMessage.get());
        SignInfo info(json::parse(wholeMessage->c_str()));
        if (info.meth() == "FORWARD") callback_->OnFORWARD(info);
        else if (info.meth() == "ACK") callback_->OnACK(info);
        else if (info.meth() == "BYE") callback_->OnBYE(info);
        else if (info.meth() == "CANCEL") callback_->OnCANCEL(info);
        else if (info.meth() == "Heartbeat") callback_->OnHeartbeat(info);
        else if (info.meth() == "OK") callback_->OnOK(info);
        else if (info.meth() == "Trying") callback_->OnTrying(info);
        else if (info.meth() == "Ringing") callback_->OnRinging(info);
        else if (info.meth() == "Unauthorized") callback_->OnUnauthorized(info);
        else W_LOG("[WSListener::readMessage] get unknown message, meth={}", info.meth());
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