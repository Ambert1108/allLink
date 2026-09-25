#include "janitorListener.hpp"
namespace rtcengine {
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // JanitorListener

    void JanitorListener::onPing(const WebSocket& socket, const oatpp::String& message) {
        I_LOG("onPing");
        std::lock_guard<std::mutex> lock(m_writeMutex);
        socket.sendPong(message);
        I_LOG("sendPong");
    }

    void JanitorListener::onPong(const WebSocket& socket, const oatpp::String& message) {
        I_LOG("onPong");
        callback_->OnPong();
    }

    void JanitorListener::onClose(const WebSocket& socket, v_uint16 code, const oatpp::String& message) {
        OATPP_LOGD(TAG, "onClose code=%d", code);
        socket.sendClose();
    }

    void JanitorListener::readMessage(const WebSocket& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) {

        if (size == 0) { // message transfer finished

            auto wholeMessage = m_messageBuffer.toString();
            m_messageBuffer.setCurrentPosition(0);
            I_LOG("on message received {}", wholeMessage->c_str());

            Message tmpMsg;
            tmpMsg.js = json::parse(wholeMessage->c_str());
            try {
                if (tmpMsg.janus() == "success") { callback_->OnSuccess(tmpMsg); }
                else if (tmpMsg.janus() == "event") { callback_->OnEvent(tmpMsg); }
                else if (tmpMsg.janus() == "ack") { callback_->OnAck(); }
                else if (tmpMsg.janus() == "webrtcup") { callback_->OnWebrtcup(); }
                else if (tmpMsg.janus() == "media") { callback_->OnMedia(); }
                else {
                    I_LOG("unknown message received : janus = [{}]", tmpMsg.janus());
                }
            }
            catch (const std::exception& e) {
                std::cerr << "A return exception: " << e.what() << std::endl;
            }
            catch (...) {
                std::cerr << "An unknown exception occurred." << std::endl;
            }

        }
        else if (size > 0) { // message frame received
            m_messageBuffer.writeSimple(data, size);
        }

    }

    void JanitorListener::registerObserver(JLObserver* callback) {
        this->callback_ = callback;
    }
}