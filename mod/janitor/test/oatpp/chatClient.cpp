#include "WSListener.hpp"
#include "oatpp-websocket/WebSocket.hpp"
#include "oatpp-websocket/Connector.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"
#include <thread>
#include <iostream>
#include <string>
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "nlohmann/json.hpp"
#include "nlohmann/fifo_map.hpp"
#include "seeker/common.h"
#include "seeker/logger.h"
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
        OATPP_LOGD(TAG, "SOCKET CLOSED!!!");
        finished = true;
    }

}

void run() {

    OATPP_LOGI(TAG, "Application Started");
    //创建连接提供者：
    ip = "10.1.29.246";
    port = 8188;
    auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({ ip, port });
    auto connector = oatpp::websocket::Connector::createShared(connectionProvider); //创建WebSocket连接器
    oatpp::websocket::Connector::Headers header;
    header.put("Sec-WebSocket-Protocol", "janus-protocol");
    auto connection = connector->connect("/", header);      //建立连接
    OATPP_LOGI(TAG, "Connected");

    auto socket = oatpp::websocket::WebSocket::createShared(connection, true);    //创建WebSocket对象：true表示客户端
    std::mutex socketWriteMutex;
    socket->setListener(std::make_shared<WSListener>(socketWriteMutex));        //设置WebSocket监听器

    std::thread thread(socketTask, socket);
    int cseq = 0;
    std::string transaction;
    std::string randomnum = std::to_string(seeker::time::currentTime());
    if (randomnum.length() >= 6) {  transaction = randomnum.substr(0, 6);   // 从位置0开始，取6个字符
    }else { transaction = randomnum; }


    json j;
    j = {
        {"janus", "create"},
        {"transaction", transaction}
    };
    std::string jsonString = j.dump();

    oatpp::String js = oatpp::String(jsonString);
    I_LOG("conversion json msg!");
    socket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
    I_LOG("send json msg!");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    I_LOG("well close!");
    socket->sendClose();
    thread.join();

}

int main(int argc, char* argv[]) {

    oatpp::base::Environment::init();
    run();
    oatpp::base::Environment::destroy();
    return 0;
}