#include "janitor.h"

namespace rtcengine {

    void Janitor::socketTask(const std::shared_ptr<oatpp::websocket::WebSocket>& websocket) {
        websocket->listen();
        I_LOG("SOCKET LISTEN CLOSED!!!"); 
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
        auto connector = oatpp::websocket::Connector::createShared(connectionProvider);
        oatpp::websocket::Connector::Headers header;
        header.put("Sec-WebSocket-Protocol", "janus-protocol");
        auto connection = connector->connect("/", header);
        I_LOG("Connected");
        socket = oatpp::websocket::WebSocket::createShared(connection, true);
        socket->setListener(listener);

        std::thread loop1{ &Janitor::socketTask,this, socket };
        listenThread = std::move(loop1);
    }

    Janitor::~Janitor() {
        I_LOG("well close!");
        destory.store(true);
        if (pingThread.joinable()) pingThread.join();
        if (aliveThread.joinable()) aliveThread.join();
        if (reconnectThread.joinable()) reconnectThread.join();
        if (socket) {
            socket->sendClose();
            socket->stopListening();
        }
        if (listenThread.joinable()) listenThread.join();
        socket = nullptr;
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

        std::thread loop2{ &Janitor::keepAlive, this };
        aliveThread = std::move(loop2);

        std::thread loop3{ &Janitor::sendPing, this };
        pingThread = std::move(loop3);
    }

    void Janitor::addNoSIP() {
        if (handleID > 0) {
            json j;
            j = {
                {"janus", "detach"},
                {"transaction", std::to_string(transaction++)},
                {"session_id", sessionID},
                {"handle_id", handleID}
            };
            std::string jsonString = j.dump(4);
            I_LOG("send message: {}", jsonString);
            oatpp::String js = oatpp::String(jsonString);
            socket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
        }


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

    void Janitor::sendTrickleToJanus(const std::string& ice) {

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

    void Janitor::sendTrickleCompleteToJanus() {

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

    void Janitor::keepAlive() {
        I_LOG("keep alive thread start");
        do {
            if (engineState.load() >= WAIT) {
                json j;
                j = {
                    {"janus", "keepalive"},
                    {"transaction", std::to_string(transaction++)},
                    {"session_id", sessionID}
                };
                //lastliveNum = transaction;
                std::string jsonString = j.dump();
                oatpp::String js = oatpp::String(jsonString);
                socket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        } while (!destory.load());
        I_LOG("keep alive thread stop");
    }

    void Janitor::sendPing() {
        receivePongTime = seeker::time::currentTime();
        while (!destory.load()) {
            socket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_PING, "Ping");
            int64_t sendPingTime = seeker::time::currentTime();
            //I_LOG("sendPingTime - receivePongTime: {}", sendPingTime - receivePongTime);
            if (sendPingTime - receivePongTime > 3000) {
                I_LOG("connect is stop ");
                socket->stopListening();
                destory.store(true);
                I_LOG("will reConnect");
                if (aliveThread.joinable()) aliveThread.join();
                if (reconnectThread.joinable()) reconnectThread.join();
                std::thread loop{ &Janitor::reconnect,this };
                reconnectThread = std::move(loop);
                break;
            }
            else {
                Sleep(1000);
            }
        }
    }

    void Janitor::reconnect() {
        I_LOG("reConnect start");
        socket = nullptr;
        engineState.store(OFFLINE);

        auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({ ip, port });
        auto connector = oatpp::websocket::Connector::createShared(connectionProvider);
        oatpp::websocket::Connector::Headers header;
        header.put("Sec-WebSocket-Protocol", "janus-protocol");
        while (1) {//尝试一直连接，直到连接成功
            std::this_thread::sleep_for(std::chrono::seconds(1));
            try {
                auto connection = connector->connect("/", header);
                if (connection) {
                    socket = oatpp::websocket::WebSocket::createShared(connection, true);
                    I_LOG("Connected");
                    break;
                }
                else continue;
            }
            catch (const std::exception& e) {
                std::cerr << "Connection failed: " << e.what() << std::endl;
            }
            catch (...) {
                std::cerr << "Unknow error"<< std::endl;
            }
        }
        sessionID = 0;
        handleID = 0;
        destory.store(false);
        engineState.store(ONLINE);
        if (pingThread.joinable()) pingThread.join();
        if (listenThread.joinable()) listenThread.join();
        socket->setListener(listener);
        std::thread loop1{ &Janitor::socketTask,this, socket };
        listenThread = std::move(loop1);
        init();

        std::thread loop2{ &Janitor::callbackRecon,this };
        loop2.detach();
        I_LOG("reConnect stop");
    }

    void Janitor::callbackRecon() {

        callback_->OnReconnect();   //通知终端
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
            if (Resp.js.contains("data") && Resp.js["data"].contains("id")) {
                handleID = Resp.data_id();
                I_LOG("handle id is [{}]", handleID);
                engineState.store(WAIT);
            }
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

    void Janitor::OnAck() {
    }

    void Janitor::OnWebrtcup() {
        I_LOG("ICE exchange success");
    }

    void Janitor::OnMedia() {
        I_LOG("media stream message");
    }

    void Janitor::OnPong() {
        //I_LOG("is connecting ...");
        receivePongTime = seeker::time::currentTime();
    }
}