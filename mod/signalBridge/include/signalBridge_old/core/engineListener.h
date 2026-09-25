#ifndef WSListener_hpp
#define WSListener_hpp

#pragma onceonce
#include <iostream>
#include "oatpp-websocket/ConnectionHandler.hpp"
#include "oatpp-websocket/WebSocket.hpp"
#include "seeker/json.hpp"
#include "nlohmann/fifomap.hpp"
#include "seeker/loggerApi.h"
#include <queue>
#include <Windows.h>

template<class K, class V, class dummy_compare, class A>
using my_workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<my_workaround_fifo_map>;

class ConnectEngineObserver {
public:
    virtual void receiveCreateSessionResp(long long session_id) = 0;
    virtual void receiveHandleResp(long long handle_id) = 0;
    virtual void receiveGenerated(std::string type, std::string sdp) = 0;
    virtual void receiveProcessed(std::string type, std::string jsep) = 0;
    virtual void receivePongResp() = 0;
protected:
    virtual ~ConnectEngineObserver() {}
};

/**
 * WebSocket listener listens on incoming WebSocket events.
 */
class EngineListener : public oatpp::websocket::WebSocket::Listener {
public:

    EngineListener(std::mutex& writeMutex);


    void RegisterObserver(ConnectEngineObserver* callback);

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

    std::atomic<bool> analysisStop = false;

private:
    void analysisResp();

    static constexpr const char* TAG = "Listener";

    std::mutex& m_writeMutex;

    std::mutex respQueMtx;

    std::queue<json> responseQueue;

    /**
     * Buffer for messages. Needed for multi-frame messages.
     */
    oatpp::data::stream::BufferOutputStream m_messageBuffer;

    ConnectEngineObserver* callback = nullptr;

    std::thread analysisThread;
};

#endif // WSListener_hpp