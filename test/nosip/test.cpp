#include <iostream>

#include "seeker/logger.h"
#include "seeker/loggerApi.h"
#include "seeker/json.hpp"
#include "nlohmann/fifomap.hpp"
#include "wslistener.h"

#include "oatpp-websocket/WebSocket.hpp"
#include "oatpp-websocket/Connector.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"

template<class K, class V, class dummy_compare, class A>
using my_workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<my_workaround_fifo_map>;

namespace {
    const char* TAG = "nosip";

    std::string JanusIp = "";
    v_uint16 JanusPort = 0;

    bool finished = false;

    void socketTask(const std::shared_ptr<oatpp::websocket::WebSocket>& websocket) {
        websocket->listen();
        finished = true;
    }

}

void JanusRun(){
    OATPP_LOGI(TAG, "start connect to Janus");
    auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({JanusIp, JanusPort});
    I_LOG("{} : {}", JanusIp, JanusPort);
    OATPP_LOGI(TAG, "start connect to Janus 1");
    auto connector = oatpp::websocket::Connector::createShared(connectionProvider);
    OATPP_LOGI(TAG, "start connect to Janus 2");
    oatpp::websocket::Connector::Headers header;
    header.put("Sec-WebSocket-Protocol", "janus-protocol");
    auto connection = connector->connect("/", header);
    OATPP_LOGI(TAG, "Janus connected done!");

    auto JanusSocket = oatpp::websocket::WebSocket::createShared(connection, true);
    JanusSocket->setListener(std::make_shared<alllink::WSListener>());
    std::thread thread(socketTask, JanusSocket);
    json j;
    j["janus"] = "create";
    j["transaction"] = "1234564";

    oatpp::String registerJson = oatpp::String(j.dump());

    JanusSocket->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, registerJson);

    thread.join();
}

int main(){
    oatpp::base::Environment::init();
    JanusIp = "10.1.29.246";
    JanusPort = 8188;

    JanusRun();

    oatpp::base::Environment::destroy();

    return 0;
}