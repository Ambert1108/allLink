#include "janus.h"
#include "seeker/json.hpp"

namespace alllink {
  JanusInteractionSystem::JanusInteractionSystem() {
    listener = std::make_shared<JanusListener>();
    listener->registerObserver(this);
    listenBody = aom::InvokeTimer::CreateTimer(std::chrono::milliseconds(1), true, [&] {
      if (client) client->listen();
      });
    keepBody = aom::InvokeTimer::CreateTimer(std::chrono::seconds(10), true, [&] {
      if (client && state >= WAIT) {
        JanusKeepAlive keep;
        keep.session_id = sessionInfo.seeionId_;
        keep.transaction = std::to_string(cseq_++);
        oatpp::String js = oatpp::String(seeker::json::toJsonString(keep));
        //TODO:测试多线程下wsclient发消息是否需要上锁
        client->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
      }
      });
    listenBody->Start();
    keepBody->Start();
  }

  JanusInteractionSystem::~JanusInteractionSystem() {
    if (client) {
      client->sendClose();
      client->stopListening();
      client = nullptr;
    }
    state = State::SESSIONING;
    if (listenBody) listenBody->Cancel();
    if (keepBody) keepBody->Cancel();
  }

  bool JanusInteractionSystem::isConnected() const {
    return true;
  }

  void JanusInteractionSystem::registerObserver(JanusInteractionObserver* callback) {
    callback_ = callback;
  }

  bool JanusInteractionSystem::connectServer(const ServerInfo& info) {
    if (info.serverIp_.empty() || info.serverPort_ < 8000) return false;
    if (serverInfo == info) {
      W_LOG("[SignlingInteractionSystem::connectServer] server addr {}:{} is same",
        info.serverIp_, info.serverPort_);
      return false;
    }

    try {
      if (client) {
        I_LOG("[JanusInteractionSystem::connectServer] server {}:{} is login", serverInfo.serverIp_, serverInfo.serverPort_);
        return false;
      }
      I_LOG("[debug] janus server is {}:{}", info.serverIp_, info.serverPort_);
      serverInfo = info;
      auto connectionProvider =
        oatpp::network::tcp::client::ConnectionProvider::createShared({ serverInfo.serverIp_, serverInfo.serverPort_ });
      auto connector = oatpp::websocket::Connector::createShared(connectionProvider);
      oatpp::websocket::Connector::Headers header;
      header.put("Sec-WebSocket-Protocol", "janus-protocol");
      auto connection = connector->connect("/", header);
      client = oatpp::websocket::WebSocket::createShared(connection, true);
      client->setListener(listener);
      JanusCreate create;
      create.transaction = std::to_string(cseq_++);
      oatpp::String js = oatpp::String(seeker::json::toJsonString(create));
      //TODO:测试多线程下wsclient发消息是否需要上锁
      if (!client->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js)) {
        E_LOG("[JanusInteractionSystem::connectServer] create session failed");
      }
    }
    catch (std::exception& ex) {
      E_LOG("[JanusInteractionSystem::connectServer] connect sever failed:{}", ex.what());
      return false;
    }
    return true;
  }

  bool JanusInteractionSystem::sendTrckileToJanus(const std::string& ice) {
    Trickle trickle;
    trickle.session_id = sessionInfo.seeionId_;
    trickle.handle_id = sessionInfo.handleId_;
    trickle.transaction = std::to_string(cseq_++);
    seeker::json::fromJsonString(trickle.candidate, ice);
    oatpp::String js = oatpp::String(seeker::json::toJsonString(trickle));
    //TODO:测试多线程下wsclient发消息是否需要上锁
    return client->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
  }

  bool JanusInteractionSystem::sendTrckileCompleteToJanus() {
    TrickleComplete trickle;
    trickle.session_id = sessionInfo.seeionId_;
    trickle.handle_id = sessionInfo.handleId_;
    trickle.transaction = std::to_string(cseq_++);
    oatpp::String js = oatpp::String(seeker::json::toJsonString(trickle));
    //TODO:测试多线程下wsclient发消息是否需要上锁
    return client->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
  }

  bool JanusInteractionSystem::sendGenerateToJanus(const std::string& jsepSdp, const std::string& type) {
    JanusGenerate generate;
    generate.session_id = sessionInfo.seeionId_;
    generate.handle_id = sessionInfo.handleId_;
    generate.transaction = std::to_string(cseq_++);
    generate.jsep.sdp = jsepSdp;
    generate.jsep.type = type;
    oatpp::String js = oatpp::String(seeker::json::toJsonString(generate));
    //TODO:测试多线程下wsclient发消息是否需要上锁
    return client->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
  }

  bool JanusInteractionSystem::sendProcessToJanus(const std::string& sdp, const std::string& type) {
    JanusProcess process;
    process.session_id = sessionInfo.seeionId_;
    process.handle_id = sessionInfo.handleId_;
    process.transaction = std::to_string(cseq_++);
    process.body.sdp = sdp;
    process.body.type = type;
    oatpp::String js = oatpp::String(seeker::json::toJsonString(process));
    //TODO:测试多线程下wsclient发消息是否需要上锁
    return client->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
  }

  void JanusInteractionSystem::OnSuccess(const JanusReponse& resp) {
    if (state == State::SESSIONING) {
      sessionInfo.seeionId_ = resp.data.id;
      JanusAttach attach;
      attach.plugin = "janus.plugin.nosip";
      attach.session_id = sessionInfo.seeionId_;
      attach.transaction = std::to_string(cseq_++);
      I_LOG("send message:{}", seeker::json::toJsonString(attach));
      oatpp::String js = oatpp::String(seeker::json::toJsonString(attach));
      //TODO:测试多线程下wsclient发消息是否需要上锁
      if (!client->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js)) {
        E_LOG("[JanusInteractionSystem::OnSuccess] create nosip failed");
        return;
      }
      state = State::ATTACHING;
    }
    else if (state == State::ATTACHING) {
      sessionInfo.handleId_ = resp.data.id;
      state = State::WAIT;
    }
  }

  void JanusInteractionSystem::OnAck(const JanusReponse& resp) {

  }

  void JanusInteractionSystem::OnEvent(const JanusReponse& resp) {

  }
}