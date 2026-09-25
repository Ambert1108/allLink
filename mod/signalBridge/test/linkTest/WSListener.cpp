#include "WSListener.hpp"

// engineListener

void engineListener::onPing(const WebSocket& socket, const oatpp::String& message) {
    OATPP_LOGD(TAG, "onPing");
    std::lock_guard<std::mutex> lock(m_writeMutex);
    socket.sendPong(message);
}

void engineListener::onPong(const WebSocket& socket, const oatpp::String& message) {
    OATPP_LOGD(TAG, "onPong");
}

void engineListener::onClose(const WebSocket& socket, v_uint16 code, const oatpp::String& message) {
    OATPP_LOGD(TAG, "onClose code=%d", code);
    socket.sendClose();
}

void engineListener::readMessage(const WebSocket& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) {

    if (size == 0) { // message transfer finished

        auto wholeMessage = m_messageBuffer.toString();
        m_messageBuffer.setCurrentPosition(0);

        OATPP_LOGD(TAG, "on message received '%s'", wholeMessage->c_str());

        /* Send message in reply */
        //std::lock_guard<std::mutex> lock(m_writeMutex);
        //socket.sendOneFrameText( "Hello from oatpp!: " + wholeMessage);

    }
    else if (size > 0) { // message frame received
        m_messageBuffer.writeSimple(data, size);
    }

}