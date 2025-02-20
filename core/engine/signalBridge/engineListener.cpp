#include "engineListener.h"

EngineListener::EngineListener(std::mutex &writeMutex) : m_writeMutex(writeMutex) {
//    std::thread thread(&EngineListener::analysisResp, this);
//    analysisThread = std::move(thread);
}

void EngineListener::RegisterObserver(ConnectEngineObserver *callback_) {
    callback = callback_;
}

//
// oatpp::websocket::WebSocket::Listener
//
void EngineListener::onPing(const WebSocket &socket, const oatpp::String &message) {
    I_LOG("onPing");
    socket.sendPong(message);
}

void EngineListener::onPong(const WebSocket &socket, const oatpp::String &message) {
    I_LOG("onPong");
    //callback->receivePongResp();
}

void EngineListener::onClose(const WebSocket &socket, v_uint16 code, const oatpp::String &message) {
    I_LOG("onClose: {}", code);
}

void EngineListener::readMessage(const WebSocket &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) {
    try {
        if (size == 0) { // message transfer finished

            auto wholeMessage = m_messageBuffer.toString();

            if (!wholeMessage->empty()) {
                m_messageBuffer.setCurrentPosition(0);

                Message tmp;
                tmp.js = json::parse(wholeMessage->c_str());
                if(tmp.cmeth() == "Heartbeat") {
                    D_LOG("readMessage: {}", tmp.js.dump(4));
                    callback->onHeartbeatResp();
                }
                else{
                    I_LOG("readMessage: {}", tmp.js.dump(4));
                }
                if (tmp.statuscode() == 200) { callback->onOK(tmp); }
                else if (tmp.statuscode() == 100) { callback->onTrying(tmp); }
                else if (tmp.statuscode() == 180) { callback->onRinging(tmp); }
                else if (tmp.statuscode() == 401) { callback->onUnauthorized(tmp); }

            }
        } else if (size > 0) { // message frame received
            m_messageBuffer.writeSimple(data, size);
        }
    }
    catch (...) {
        E_LOG("readMessage error");
    }
}


//void EngineListener::analysisResp() {
//    while (!analysisStop) {
//        if (!responseQueue.empty()) {
//            I_LOG("before responseQueue.size: {}", responseQueue.size());
//            std::unique_lock<std::mutex> lk(respQueMtx);
//            Message tmp = responseQueue.front();
//            lk.unlock();
//
//            std::unique_lock<std::mutex> lk2(respQueMtx);
//            responseQueue.pop();
//            I_LOG("after responseQueue.size: {}", responseQueue.size());
//            lk2.unlock();
//        } else {
//            Sleep(20);
//        }
//    }
//}
