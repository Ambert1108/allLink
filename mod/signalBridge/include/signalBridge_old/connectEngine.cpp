//
// Created by 姚惠晶 on 2024/12/3.
//

#include "connectEngine.h"

ConnectEngine::~ConnectEngine(){
    D_LOG("~ConnectEngine");
    threadDestroy = true;
    if(keepaliveThread.joinable()) {
        keepaliveThread.join();
    }
    JanusSocket->stopListening();
    if(listenerThread.joinable()) {
        listenerThread.join();
    }
}

void ConnectEngine::connectToJanus(std::string JanusIp, uint16_t JanusPort) {
    try {
        D_LOG("start connect to Janus");

        auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({JanusIp, JanusPort});
        D_LOG("Janus: {}:{}", JanusIp, JanusPort);
        auto connector = oatpp::websocket::Connector::createShared(connectionProvider);
        oatpp::websocket::Connector::Headers header;
        header.put("Sec-WebSocket-Protocol", "janus-protocol");
        auto connection = connector->connect("/", header);
        D_LOG("signaling connected done!");

        WS_CONNECT = true;
        JanusSocket = nullptr;
        JanusSocket = oatpp::websocket::WebSocket::createShared(connection,true /* maskOutgoingMessages must be true for clients */);

        if(first) {
            listener = std::make_shared<EngineListener>(socketWriteMutex);
        }
        JanusSocket->setListener(listener);

        if(listenerThread.joinable()){
            listenerThread.join();
        }
        std::thread thread(&ConnectEngine::socketTask, this, JanusSocket);
        listenerThread = std::move(thread);

        listener->RegisterObserver(this);
        
        
        std::thread pingThread(&ConnectEngine::sendPing, this);
        if(first) {
            sendPingThread = std::move(pingThread);
        }
        else{
            threadDestroy = false;
            pingThread.detach();
        }

        if(first) {
            first = false;
        }
        else{
            listener->analysisStop = false;
            OnWSReConnect();
        }

    }
    catch (const std::exception& e){
        E_LOG("connectToJanus error: {}", e.what());
    }
    catch (...){
        E_LOG("connectToJanus error");
    }
}

int ConnectEngine::createSession() {
    D_LOG("createSession");
    json create;
    std::string transaction = std::to_string(rand());
    create["janus"] = "create";
    create["transaction"] = transaction;
    D_LOG("create Req: {}", create.dump(4));
    oatpp::String createJson = oatpp::String(create.dump());
    JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, createJson);
    int64_t t1 = seeker::Time::currentTime();
    while(session_id == -1){
        Sleep(1);
        int64_t t2 = seeker::Time::currentTime();
        if(t2-t1 > 5000){
            E_LOG("obtain session_id error, session_id: {}", session_id);
            return -1;
        }
    }
    std::thread thread(&ConnectEngine::keepalive, this);
    keepaliveThread = std::move(thread);

    return 0;
}

int ConnectEngine::attachNoSIP() {
    D_LOG("attachNoSIP");
    if(session_id != -1){
        if(handle_id != -1){
            json detach;
            detach["janus"] = "attach";
            detach["session_id"] = session_id;
            detach["handle_id"] = handle_id;
            detach["transaction"] = std::to_string(rand());
        }
        json attach;
        attach["janus"] = "attach";
        attach["plugin"] = "janus.plugin.nosip";
        attach["opaque_id"] = "user" + std::to_string(rand());
        attach["session_id"] = session_id;
        attach["transaction"] = std::to_string(rand());
        D_LOG("attach Req: {}", attach.dump(4));
        oatpp::String attachJson = oatpp::String(attach.dump());
        JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, attachJson);
        int64_t t1 = seeker::Time::currentTime();
        while (handle_id == -1){
            Sleep(1);
            int64_t t2 = seeker::Time::currentTime();
            if(t2-t1 > 5000){
                E_LOG("obtain handle_id error, handle_id: {}", handle_id);
                return -1;
            }
        }
    }
    else{
        E_LOG("please createSession first, session_id: {}",session_id);
        return -1;
    }

    return 0;
}

void ConnectEngine::sendGenerate(std::string type, std::string jsep_) {
    D_LOG("sendGenerate");
    if(session_id != -1 && handle_id != -1) {
        json generate;
        generate["janus"] = "message";
        generate["body"]["request"] = "generate";
        generate["transaction"] = std::to_string(rand());
        generate["jsep"]["type"] = type;
        generate["jsep"]["sdp"] = jsep_;
        generate["session_id"] = session_id;
        generate["handle_id"] = handle_id;
        D_LOG("generate Req: {}", generate.dump(4));
        oatpp::String generateJson = oatpp::String(generate.dump());
        JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, generateJson);
    }
    else{
        E_LOG("session_id or handle_id is NULL");
    }
}

void ConnectEngine::sendProcess(std::string type, std::string sdp_) {
    D_LOG("sendProcess");
    if(session_id != -1 && handle_id != -1) {
        json process;
        process["janus"] = "message";
        process["body"]["request"] = "process";
        process["body"]["type"] = type;
        process["body"]["sdp"] = sdp_.c_str();
        process["transaction"] = std::to_string(rand());
        process["session_id"] = session_id;
        process["handle_id"] = handle_id;
        D_LOG("process Req: {}", process.dump(4));
        oatpp::String processJson = oatpp::String(process.dump());
        JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, processJson);
    }
    else{
        E_LOG("session_id or handle_id is NULL");
    }
}

void ConnectEngine::sendTrickle(const webrtc::IceCandidateInterface *candidate) {
    json trickle;
    trickle["janus"] = "trickle";
    trickle["candidate"]["sdpMid"] = candidate->sdp_mid();
    trickle["candidate"]["sdpMLineIndex"] = candidate->sdp_mline_index();

    std::string sdp;
    if (!candidate->ToString(&sdp)) {
        E_LOG("Failed to serialize candidate");
        return;
    }

    trickle["candidate"]["candidate"] = sdp;
    trickle["transaction"] = std::to_string(rand());
    trickle["session_id"] = session_id;
    trickle["handle_id"] = handle_id;
    D_LOG("trickle Req: {}", trickle.dump(4));
    oatpp::String trickleJson = oatpp::String(trickle.dump());
    JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, trickleJson);
}

void ConnectEngine::sendTrickleComplete() {
    json trickle;
    trickle["janus"] = "trickle";
    trickle["candidate"]["completed"] = true;
    trickle["transaction"] = std::to_string(rand());
    trickle["session_id"] = session_id;
    trickle["handle_id"] = handle_id;
    D_LOG("trickle Req: {}", trickle.dump(4));
    oatpp::String trickleJson = oatpp::String(trickle.dump());
    JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, trickleJson);
}

//
// private
//
void ConnectEngine::keepalive() {
    while(!threadDestroy){
        json keepalive;
        keepalive["janus"] = "keepalive";
        keepalive["session_id"] = session_id;
        keepalive["transaction"] = std::to_string(rand());
        D_LOG("keepalive Req: {}", keepalive.dump(4));
        oatpp::String keepaliveJson = oatpp::String(keepalive.dump());
        if(!threadDestroy) {
            JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, keepaliveJson);
        }
        Sleep(5000);
    }
}

void ConnectEngine::sendPing() {
    receivePongTime = time_s::currentTime();
    while(!threadDestroy) {
        JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_PING, "Ping");
        sendPingTime = time_s::currentTime();
        D_LOG("sendPingTime - receivePongTime: {}", sendPingTime - receivePongTime);
        if(sendPingTime - receivePongTime > 2000){
            threadDestroy = true;
            listener->analysisStop = true;

            if(keepaliveThread.joinable()) {
                keepaliveThread.join();
                D_LOG("keepaliveThread join");
            }
            JanusSocket->stopListening();

            WS_CONNECT = false;
            session_id = -1;
//            handle_id = -1;   // 需要根据handle_id是否为空来判断是否进行detach
            OnLoseWSConnect();
            return;
        }
        else {
            Sleep(1000);
        }
    }
}

void ConnectEngine::socketTask(const std::shared_ptr<oatpp::websocket::WebSocket> &websocket) {
    try {
        websocket->listen();
    }
    catch (...){
        E_LOG("websocket->listen error");
    }
}

//
// ConnectObserver
//
void ConnectEngine::receiveCreateSessionResp(long long session_id_) {
    session_id = session_id_;
    D_LOG("session_id: {}", session_id);
}

void ConnectEngine::receiveHandleResp(long long handle_id_) {
    handle_id = handle_id_;
    D_LOG("session[{}], handle_id: {}", session_id, handle_id);
}

void ConnectEngine::receiveGenerated(std::string type_, std::string sdp_) {
    OnGenerated(type_, sdp_);
}

void ConnectEngine::receiveProcessed(std::string type_, std::string jsep_) {
    OnProcessed(type_, jsep_);
}

void ConnectEngine::receivePongResp() {
   receivePongTime = time_s::currentTime();
}