#include "signling.h"

#include "api/units/time_delta.h"
#include "rtc_base/async_dns_resolver.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/net_helpers.h"
#include "rtc_base/thread.h"


namespace alllink {
  SignlingInteractionSystem::SignlingInteractionSystem() {
    oatpp::base::Environment::init();
    listener = std::make_shared<WSListener>();
    listener->registerObserver(this);
    listenBody = aom::InvokeTimer::CreateTimer(std::chrono::milliseconds(1), true, [&] {
      if (signalState > 1 && client) client->listen();
    });
    listenBody->Start();
  }

  SignlingInteractionSystem::~SignlingInteractionSystem() {
    if(client) logout();
    if(listenBody) listenBody->Cancel();
    oatpp::base::Environment::destroy();
  }

  bool SignlingInteractionSystem::isConnected() const {
    return true;
  }

  void SignlingInteractionSystem::registerObserver(SignlingInteractionObserver* callback) {
    callback_ = callback;
  }

  bool SignlingInteractionSystem::connectServer(const LinkInfo& info) {
    if (info.serverIp_.empty() || info.serverPort_ < 8888) return false;
    if (linkInfo == info) {
      W_LOG("[SignlingInteractionSystem::connectServer] server addr {}:{} is same",
        info.serverIp_, info.serverPort_);
      return false;
    }

    try {
      if (client) {
        logout();
        I_LOG("[SignlingInteractionSystem::connectServer] login out! old server is {}:{}, new server is {}:{}",
          linkInfo.serverIp_, linkInfo.serverPort_, info.serverIp_, info.serverPort_);
      }

      linkInfo = info;
      auto connectionProvider =
        oatpp::network::tcp::client::ConnectionProvider::createShared({ linkInfo.serverIp_, linkInfo.serverPort_ });
      auto connector = oatpp::websocket::Connector::createShared(connectionProvider);
      auto connection = connector->connect("/connectWS");
      client = oatpp::websocket::WebSocket::createShared(connection, true);
      client->setListener(listener);
      signalState = State::LOGIN_ON;
    }
    catch (std::exception& ex) {
      E_LOG("[SignlingInteractionSystem::connectServer] connect sever failed:{}", ex.what());
      return false;
    }
    return true;
  }

  bool SignlingInteractionSystem::login(const UserInfo& info) {
    userInfo = info;
    SignInfo msg;
    msg.set_meth("REGISTER");
    msg.set_isresponse(false);
    msg.set_cseq(cseq_);
    msg.set_userid(info.id_);
    msg.set_password(info.pwd_);
    return ToREGISTER(msg);
  }

  void SignlingInteractionSystem::logout() {
    client->sendClose();
    client->stopListening();
    client = nullptr;
    signalState = State::LOGIN_OUT;
  }

  void SignlingInteractionSystem::OnFORWARD(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnACK(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnBYE(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnCANCEL(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnHeartbeat(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnOK(const SignInfo& info) {
    if (info.cmeth() == "FORWARD") {
      //收到信令转发主叫呼叫请求
    }
    else if (info.cmeth() == "REGISTER") {
      //收到信令回复登录请求
      hi::PostMsg({ msgTo(MessageType::LOGIN_SUCCESS), info.to() });
    }
  }

  void SignlingInteractionSystem::OnTrying(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnRinging(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnUnauthorized(const SignInfo& info) {

  }


  //private
  bool SignlingInteractionSystem::ToREGISTER(const SignInfo& info) {
    oatpp::String js = oatpp::String(info.js.dump());
    return client->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
  }

  bool SignlingInteractionSystem::ToFORWARD(const SignInfo& info) {

  }

  bool SignlingInteractionSystem::ToACK(const SignInfo& info) {

  }

  bool SignlingInteractionSystem::ToBYE(const SignInfo& info) {

  }

  bool SignlingInteractionSystem::ToCANCEL(const SignInfo& info) {

  }

  bool SignlingInteractionSystem::ToINFO(const SignInfo& info) {

  }

  bool SignlingInteractionSystem::ToHeartbeat(const SignInfo& info) {

  }

  bool SignlingInteractionSystem::ToOK(const SignInfo& info) {

  }

  bool SignlingInteractionSystem::ToTrying(const SignInfo& info) {

  }

  bool SignlingInteractionSystem::ToRinging(const SignInfo& info) {

  }


}
