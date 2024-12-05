#include "janitor.h"

void Janitor::socketTask(const std::shared_ptr<oatpp::websocket::WebSocket>& websocket) {
    websocket->listen();
    I_LOG("SOCKET CLOSED!!!");
}

Janitor::Janitor(std::string ip, v_uint16 port) {
    this->ip = ip;
    this->port = port;    
    std::mutex socketWriteMutex;
    listener = std::make_shared<JanitorListener>(socketWriteMutex);
    listener->registerObserver(this);
    I_LOG("creat listener");
    //创建连接提供者：
    auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({ ip, port });
    auto connector = oatpp::websocket::Connector::createShared(connectionProvider); //创建WebSocket连接器
    oatpp::websocket::Connector::Headers header;
    header.put("Sec-WebSocket-Protocol", "janus-protocol");
    auto connection = connector->connect("/", header);      //建立连接
    I_LOG("Connected");


    socket = oatpp::websocket::WebSocket::createShared(connection, true);    //创建WebSocket对象：true表示客户端
    socket->setListener(listener);		//设置WebSocket监听器
    std::thread loop1{ &Janitor::socketTask,this, socket };
    listenThread = std::move(loop1);
}

Janitor::~Janitor() {
    I_LOG("well close!");
    if (socket) {
      socket->sendClose();
      socket->stopListening();
      socket = nullptr;
    }
    destory.store(true);
    if(aliveThread.joinable()) aliveThread.join();
    if (listenThread.joinable()) listenThread.join();
    I_LOG("janitor close success");
}

void Janitor::init() {

    json j; 
    j = {
        {"janus", "create"},
        {"transaction", std::to_string(transaction++)}
    };
    std::string jsonString = j.dump(4);
    I_LOG("send message: {}", jsonString);
    oatpp::String js = oatpp::String(jsonString);
    socket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
    engineState.store(SESSIONING);

    //do{
    //    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    //} while ();

    std::thread loop2{ &Janitor::keepAlive, this };
    aliveThread = std::move(loop2);
}

void Janitor::generateSDP(std::string jsepSdp, std::string type) {
    json j;
    j = {
        {"janus", "message"},
        {"transaction", std::to_string(transaction++)},
        {"session_id", sessionID},
        {"handle_id", handleID},
        {"body", {
            {"request", "generate"}}},
        {"jsep", {
            {"sdp",jsepSdp},
            {"type", type} }}
    };
    std::string jsonString = j.dump(4);
    I_LOG("send message: {}", jsonString);
    oatpp::String js = oatpp::String(jsonString);
    socket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
    engineState.store(GENERATING);
}

void Janitor::processSDP(std::string normalSdp, std::string type) {
    json j;
    j = {
        {"janus", "message"},
        {"transaction", std::to_string(transaction++)},
        {"session_id", sessionID},
        {"handle_id", handleID},
        {"body", {
            {"request", "process"},
            {"type", type},
            {"sdp", normalSdp} }}
    };
    std::string jsonString = j.dump(4);
    I_LOG("send message: {}", jsonString);
    oatpp::String js = oatpp::String(jsonString);
    socket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
    engineState.store(PROCESSING);
}

void Janitor::sendTrckileToJanus(const std::string& ice) {
   
    json icedata = json::parse(ice);
    json j;
    j = {
        {"janus", "trickle"},
        {"transaction", std::to_string(transaction++)},
        {"session_id", sessionID},
        {"handle_id", handleID}
    };
    j["candidate"]["sdpMid"] = icedata["sdpMid"];
    j["candidate"]["sdpMLineIndex"] = icedata["sdpMLineIndex"];
    j["candidate"]["candidate"] = icedata["candidate"];

    std::string jsonString = j.dump(4);
    I_LOG("send message: {}", jsonString);
    oatpp::String js = oatpp::String(jsonString);
    socket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
}

void Janitor::sendTrckileCompleteToJanus() {

    json j;
    j = {
        {"janus", "trickle"},
        {"transaction", std::to_string(transaction++)},
        {"session_id", sessionID},
        {"handle_id", handleID},
        {"candidate", {
          {"completed", true}}
        }
    };
    std::string jsonString = j.dump(4);
    I_LOG("send message: {}", jsonString);
    oatpp::String js = oatpp::String(jsonString);
    socket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
}

int64_t Janitor::getSessionId() const {
    return sessionID;
}

int64_t Janitor::getHandleId() const {
    return handleID;
}

void Janitor::keepAlive() {
  I_LOG("keep alive thread start");
    do {
      if (engineState.load() >= WAIT) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        json j;
        j = {
            {"janus", "keepalive"},
            {"transaction", std::to_string(transaction++)},
            {"session_id", sessionID}
        };
        std::string jsonString = j.dump();
        oatpp::String js = oatpp::String(jsonString);
        socket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } while (!destory.load());
    I_LOG("keep alive thread stop");
}

void Janitor::addNoSIP() {

    json j;
    j = {
        {"janus", "attach"},
        {"transaction", std::to_string(transaction++)},
        {"session_id", sessionID},
        {"plugin", "janus.plugin.nosip"}
    };
    std::string jsonString = j.dump(4);
    I_LOG("send message: {}", jsonString);
    oatpp::String js = oatpp::String(jsonString);
    socket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
    engineState.store(HANDLEING);

}



void Janitor::registerObserver(JanitorObserver* callback) {
    this->callback_ = callback;
}

void Janitor::OnSuccess(const Message& message) {

    Message Resp = message;
    if (engineState.load() == SESSIONING) {
        sessionID = Resp.data_id();
        I_LOG("session id is [{}]", sessionID);

        addNoSIP();
    }
    else if (engineState.load() == HANDLEING) {
        handleID = Resp.data_id();
        I_LOG("handle id is [{}]", handleID);
        engineState.store(WAIT);
    }

}

void Janitor::OnEvent(const Message& message) {

    Message Resp = message;
    if (Resp.plugindata_data_result_event() == "generated") {
        std::string normalSdp = Resp.plugindata_data_result_sdp();
        std::string normalType = Resp.plugindata_data_result_type();
        engineState.store(WAIT);
        callback_->OnGenerated(normalSdp, normalType);

    }
    else if (Resp.plugindata_data_result_event() == "processed") {
        std::string jsepSdp = Resp.jsep_sdp();
        std::string jsepType = Resp.jsep_type();
        engineState.store(WAIT);
        callback_->OnProcessed(jsepSdp, jsepType);
    }
}