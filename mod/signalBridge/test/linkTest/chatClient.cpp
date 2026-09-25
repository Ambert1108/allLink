#include "WSListener.hpp"
#include "oatpp-websocket/WebSocket.hpp"
#include "oatpp-websocket/Connector.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"
#include <thread>
#include <iostream>
#include <string>
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "seeker/json.hpp"
#include "nlohmann/fifomap.hpp"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "seeker/loggerApi.h"

template<class K, class V, class dummy_compare, class A>
using my_workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<my_workaround_fifo_map>;

namespace {

    const char* TAG = "websocket-client";

    bool finished = false;
    std::string ip = "";
    v_uint16 port = 0;
    std::string password = "";
    void socketTask(const std::shared_ptr<oatpp::websocket::WebSocket>& websocket) {
        websocket->listen();
        finished = true;
    }

}

void run(std::string username) {

    OATPP_LOGI(TAG, "Application Started");
    //创建连接提供者：
    auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({ ip, port });
    //创建WebSocket连接器：
    auto connector = oatpp::websocket::Connector::createShared(connectionProvider);
    //建立连接,通过相应路径发起WebSocket握手请求,注：确保路径是websocket连接否则会抛出runtime_erro错误
    //auto connection = connector->connect("/connectWS?username=" + username + "&password=" + password);
    auto connection = connector->connect("/connectWS");

    OATPP_LOGI(TAG, "Connected");
    //创建WebSocket对象：第二个参数true表示这是一个客户端连接，服务器端应该设置为false。
    auto socket = oatpp::websocket::WebSocket::createShared(connection, true /* maskOutgoingMessages must be true for clients */);

    std::mutex socketWriteMutex;
    //设置WebSocket监听器：
    socket->setListener(std::make_shared<engineListener>(socketWriteMutex));

    std::thread thread(socketTask, socket);
    std::string text;
    std::string call_username;
    int cseq = 0;
    I_LOG("sending msg!");
    call_username = "3185";
    I_LOG("eidt json msg!");
    std::lock_guard<std::mutex> lock(socketWriteMutex);

    json j;
    j["meth"] = "REGISTER";
    j["isResponse"] = false;
    j["userId"] = username;
    j["password"] = password;
    j["Cseq"] = cseq++;

    oatpp::String js = oatpp::String(j.dump());
    I_LOG("conversion json msg!");
    socket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
    I_LOG("send json msg!");
    //}
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //}
    I_LOG("well close!");
    socket->sendClose();
    thread.join();

}

int main(int argc, char* argv[]) {
    std::string username = "3185";
    ip = "10.1.29.247";
    port = 32103;
    password = "yhj";
    oatpp::base::Environment::init();
    run(username);
    oatpp::base::Environment::destroy();
    return 0;
}