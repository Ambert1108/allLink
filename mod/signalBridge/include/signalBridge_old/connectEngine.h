//
// Created by 姚惠晶 on 2024/12/3.
//

#ifndef SIGNALBRIDGE_CONNECTENGINE_H
#define SIGNALBRIDGE_CONNECTENGINE_H

#pragma once
#include <iostream>
#include <thread>
#include "seeker/loggerApi.h"
#include "seeker/json.hpp"
#include "nlohmann/fifomap.hpp"
#include "signalBridge/engineListener.h"
#include "seeker/common.h"

#include "oatpp-websocket/WebSocket.hpp"
#include "oatpp-websocket/Connector.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"
#include "rtc_base/win32_socket_init.h"
#include "api/make_ref_counted.h"
#include "api/peer_connection_interface.h"
#include "api/rtp_sender_interface.h"

template<class K, class V, class dummy_compare, class A>
using my_workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<my_workaround_fifo_map>;

using time_s = seeker::Time;

class ConnectEngine : ConnectEngineObserver {
public:
    ConnectEngine() {}
    ~ConnectEngine();

    void connectToJanus(std::string JanusIp, uint16_t JanusPort);
    int createSession();
    int attachNoSIP();
    void sendGenerate(std::string type, std::string jsep);
    void sendProcess(std::string type, std::string sdp);
    void sendTrickle(const webrtc::IceCandidateInterface *candidate);
    void sendTrickleComplete();

    virtual void OnGenerated(std::string type, std::string sdp) = 0;
    virtual void OnProcessed(std::string type, std::string jsep) = 0;
    virtual void OnLoseWSConnect() = 0;
    virtual void OnWSReConnect() = 0;

    std::atomic<bool> threadDestroy = false;
    std::atomic<bool> WS_CONNECT = false;
private:
    void keepalive();
    void socketTask(const std::shared_ptr<oatpp::websocket::WebSocket> &websocket);
    void sendPing();

    //
    // ConnectObserver
    //
    void receiveCreateSessionResp(long long session_id) override;
    void receiveHandleResp(long long handle_id) override;
    void receiveGenerated(std::string type, std::string sdp) override;
    void receiveProcessed(std::string type, std::string jsep) override;
    void receivePongResp() override;


    std::string JanusIp = "";

    uint16_t JanusPort = 0;

    std::shared_ptr<EngineListener> listener = nullptr;

    std::shared_ptr<oatpp::websocket::WebSocket> JanusSocket = nullptr;

    std::thread listenerThread;

    std::thread keepaliveThread;

    std::thread sendPingThread;

    std::mutex socketWriteMutex;

    long long session_id = -1;

    long long handle_id = -1;

    oatpp::data::stream::BufferOutputStream m_messageBuffer;

    int64_t sendPingTime = 0;

    int64_t receivePongTime = 0;

    bool first = true;
};


#endif //SIGNALBRIDGE_CONNECTENGINE_H
