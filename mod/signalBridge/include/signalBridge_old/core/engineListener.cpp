#include "engineListener.h"

EngineListener::EngineListener(std::mutex &writeMutex) : m_writeMutex(writeMutex) {
    std::thread thread(&EngineListener::analysisResp, this);
    analysisThread = std::move(thread);
}

void EngineListener::RegisterObserver(ConnectEngineObserver *callback_) {
    callback = callback_;
}

//
// oatpp::websocket::WebSocket::Listener
//
void EngineListener::onPing(const WebSocket &socket, const oatpp::String &message) {
    D_LOG("onPing");
    socket.sendPong(message);
}

void EngineListener::onPong(const WebSocket &socket, const oatpp::String &message) {
    D_LOG("onPong");
    callback->receivePongResp();
}

void EngineListener::onClose(const WebSocket &socket, v_uint16 code, const oatpp::String &message) {
    D_LOG("onClose: {}", code);
}

void EngineListener::readMessage(const WebSocket &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) {
    if (size == 0) { // message transfer finished

        auto wholeMessage = m_messageBuffer.toString();
        if(!wholeMessage->empty()) {
            m_messageBuffer.setCurrentPosition(0);

            OATPP_LOGD(TAG, "on message received '%s'", wholeMessage->c_str());

            json tmpJson = json::parse(wholeMessage->c_str());
            if (tmpJson["janus"] != "ack") {
                std::unique_lock<std::mutex> lk(respQueMtx);
                responseQueue.push(tmpJson);
            }
        }
    } else if (size > 0) { // message frame received
        m_messageBuffer.writeSimple(data, size);
    }
}


void EngineListener::analysisResp() {
    while (true) {
        if (!analysisStop) {
            if (!responseQueue.empty()) {
                D_LOG("responseQueue.size: {}", responseQueue.size());
                std::unique_lock<std::mutex> lk(respQueMtx);
                json tmp = responseQueue.front();
                lk.unlock();
                if (tmp["janus"] == "success") {
                    if (!tmp.contains("session_id")) {
                        if (!tmp["data"]["id"].is_null()) {
                            long long session_id = tmp["data"]["id"];
                            callback->receiveCreateSessionResp(session_id);
                        }
                    } else {
                        if (!tmp["data"]["id"].is_null()) {
                            long long handle_id = tmp["data"]["id"];
                            callback->receiveHandleResp(handle_id);
                        }
                    }
                } else if (tmp["janus"] == "event") {
                    if (!tmp["plugindata"]["data"]["result"].is_null()) {
                        if (tmp["plugindata"]["data"]["result"]["event"] == "generated") {
                            std::string type = tmp["plugindata"]["data"]["result"]["type"].get<std::string>();
                            std::string sdp = tmp["plugindata"]["data"]["result"]["sdp"].get<std::string>();
                            callback->receiveGenerated(type, sdp);
                        } else if (tmp["plugindata"]["data"]["result"]["event"] == "processed") {
                            std::string type = tmp["jsep"]["type"].get<std::string>();
                            std::string jsdp = tmp["jsep"]["sdp"].get<std::string>();
                            callback->receiveProcessed(type, jsdp);
                        }
                    }
                }
                std::unique_lock<std::mutex> lk2(respQueMtx);
                responseQueue.pop();
                lk2.unlock();
            }
        }
    }

}
