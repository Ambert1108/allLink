#include "signling.h"

#include "api/units/time_delta.h"
#include "rtc_base/async_dns_resolver.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/net_helpers.h"
#include "rtc_base/thread.h"


namespace alllink {
  SignlingInteractionSystem::SignlingInteractionSystem() {
    listener = std::make_shared<WSListener>();
    listener->registerObserver(this);
    listenBody = aom::InvokeTimer::CreateTimer(std::chrono::milliseconds(1), true, [&] {
      try {
        if (signalState > 0 && client) {
          I_LOG("signling listen start");
          client->listen();
          I_LOG("signling listen finish");
        }
      }
      catch (const std::exception& ex) {
        E_LOG("listenBody timer catch:{}", ex.what());
      }
      catch (...) {
        E_LOG("listenBody timer catch unknown exception");
      }
    });
    listenBody->Start();
    keepBody = aom::InvokeTimer::CreateTimer(std::chrono::milliseconds(1500), true, [&] {
      try {
        if (signalState > 1 && client) {
          if(lastBeatPoint == 0) lastBeatPoint = seeker::time::currentTime();
          if (seeker::time::currentTime() - lastBeatPoint > 3000) {
            W_LOG("[Signling::keepBody] heartbeat timeout 3s");
            logout();
            lastBeatPoint = 0;
            signalState = State::NONE;
            callback_->OnSignlingDisconnect();
          }
          else {
            SignInfo msg;
            msg.set_meth("Heartbeat");
            msg.set_from(userInfo.id_);
            msg.set_to("system");
            oatpp::String js = oatpp::String(msg.js.dump());
            std::unique_lock<std::mutex> lck(Locker);
            client->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
          }
        }
      }
      catch (const std::exception& ex) {
        E_LOG("keepBody timer catch:{}, start logout", ex.what());
        client->stopListening();
        client = nullptr;
        lastBeatPoint = 0;
        signalState = State::NONE;
        callback_->OnSignlingDisconnect();
      }
      catch (...) {
        E_LOG("keepBody timer catch unknown exception, start logout");
        client->stopListening();
        client = nullptr;
        lastBeatPoint = 0;
        signalState = State::NONE;
        callback_->OnSignlingDisconnect();
      }
    });
    keepBody->Start();
  }

  SignlingInteractionSystem::~SignlingInteractionSystem() {
    if(keepBody) keepBody->Cancel();
    if(client) logout();
    if(listenBody) listenBody->Cancel();
  }

  bool SignlingInteractionSystem::isConnected() const {
    return true;
  }

  void SignlingInteractionSystem::registerObserver(SignlingInteractionObserver* callback) {
    callback_ = callback;
  }

  bool SignlingInteractionSystem::connectServer(const ServerInfo& info) {
    if (info.serverIp_.empty() || info.serverPort_ < 8888) return false;
    if (serverInfo == info) {
      W_LOG("[SignlingInteractionSystem::connectServer] server addr {}:{} is same",
        info.serverIp_, info.serverPort_);
      return false;
    }
    if (!connect(info)) return false;
    serverInfo = info;
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
    return ToSignaling(msg);
  }

  void SignlingInteractionSystem::disConnectServer() {
    {
      std::unique_lock<std::mutex> lck(Locker);
      logout();
    }
    serverInfo.clear();
    userInfo.clear();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  bool SignlingInteractionSystem::reLogin() {
    if (!connect(serverInfo)) {
      serverInfo.clear();
      return false;
    }
    if (!login(userInfo)) {
      userInfo.clear();
      return false;
    }
    return true;
  }

  bool SignlingInteractionSystem::sendToPeer(const std::string& to, const std::string& message) {
    SignInfo msg;
    std::regex pattern("^\\d{3}-\\d{3}$");
    if (std::regex_match(to, pattern)) {
      // 用户进入会议流程
      mode = 1;
      I_LOG("use meeting");
      msg.set_meth("INVITE");
      msg.set_from(userInfo.id_);
      msg.set_to(to);
      msg.set_sdp(message);
      msg.set_cseq(cseq_);
      callId = userInfo.id_ + to + std::to_string(cseq_++);
      msg.set_call_id(callId);
      signalState = State::CALLING;
    }
    else{
      mode = 0;
      I_LOG("use 1v1 calling");
      // 用户进入1v1通话流程
      msg.set_meth("FORWARD");
      msg.set_from(userInfo.id_);
      msg.set_to(to);
      msg.set_sdp(message);
    }
    hi::PostMsg({ msgTo(MessageType::CALL_MODE), mode });
    return ToSignaling(msg);
  }

  void SignlingInteractionSystem::sendTrickle(const std::string& to, const Candidate& ice) {
    SignInfo msg;
    msg.set_meth("TRICKLE");
    msg.set_from(userInfo.id_);
    msg.set_to(to);
    msg.set_candidate(ice.candidate);
    msg.set_sdpMid(ice.sdpMid);
    msg.set_sdpMLineIndex(ice.sdpMLineIndex);
    msg.set_cseq(cseq_++);
    msg.set_call_id(callId);
    if (!ToSignaling(msg)) E_LOG("send msg to Signaling failed {}", msg.js.dump(4));
  }

  void SignlingInteractionSystem::sendTrickleComplete(const std::string& to) {
    SignInfo msg;
    msg.set_meth("TRICKLE");
    msg.set_from(userInfo.id_);
    msg.set_to(to);
    msg.set_completed(true);
    msg.set_cseq(cseq_++);
    msg.set_call_id(callId);
    if (!ToSignaling(msg)) E_LOG("send msg to Signaling failed {}", msg.js.dump(4));
  }

  bool SignlingInteractionSystem::sendAck(const std::string& to) {
    SignInfo msg;
    msg.set_meth("ACK");
    msg.set_from(userInfo.id_);
    msg.set_to(to);
    msg.set_cseq(cseq_++);
    msg.set_call_id(callId);
    return ToSignaling(msg);
  }

  bool SignlingInteractionSystem::sendInfo(const std::string& to, int info) {
    SignInfo msg;
    msg.set_meth("INFO");
    msg.set_from(userInfo.id_);
    msg.set_to(to);
    msg.set_cseq(cseq_++);
    msg.set_call_id(callId);
    msg.set_signal(std::to_string(info));
    return ToSignaling(msg);
  }

  bool SignlingInteractionSystem::sendBye(const std::string& to) {
    SignInfo msg;
    if (mode == 1) {
      msg.set_meth("BYE");
      msg.set_from(userInfo.id_);
      msg.set_to(to);
      msg.set_cseq(cseq_++);
      msg.set_call_id(callId);
    }
    else {
      msg.set_meth("FORWARD");
      msg.set_from(userInfo.id_);
      msg.set_to(to);
      msg.set_signal("bye");
      msg.set_cseq(cseq_++);
      msg.set_call_id(callId);
    }
    return ToSignaling(msg);
  }

  void SignlingInteractionSystem::logout() {
    client->sendClose();
    client->stopListening();
    client = nullptr;
    lastBeatPoint = 0;
    signalState = State::NONE;
  }

  void SignlingInteractionSystem::OnFORWARD(const SignInfo& info) {
    I_LOG("On FORWARD");
    if (!info.signal().empty()) {
      I_LOG("on peer disconnect");
      // 收到对端发来bye请求，关闭peerConnection
      callback_->OnPeerDisconnected(info.from());
    }
    else {
      I_LOG("on message from signling");
      callback_->OnMessageFromSignling(info);
    }
    I_LOG("On FORWARD finish");
  }

  void SignlingInteractionSystem::OnACK(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnBYE(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnCANCEL(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnHeartbeat(const SignInfo& info) {
    lastBeatPoint = seeker::time::currentTime();
  }

  void SignlingInteractionSystem::OnOK(const SignInfo& info) {
    if (info.cmeth() == "FORWARD") {
      //收到信令转发主叫呼叫请求
    }
    else if (info.cmeth() == "REGISTER") {
      signalState = State::LOGIN_ON;
      //收到信令回复登录请求
      I_LOG("login success 1");
      hi::PostMsg({ msgTo(MessageType::LOGIN_SUCCESS), info.to() });
    }
    else if (info.cmeth() == "INVITE") {
      SignInfo msg;
      msg.set_from(info.from());
      Jsep jsep;
      jsep.sdp = info.sdp();
      jsep.type = "answer";
      std::string sdp = seeker::json::toJsonString(jsep);
      msg.set_sdp(sdp);
      callback_->OnMessageFromSignling(msg);
      signalState = State::CALLER;
      if (info.timePoint() == -1) {
        hi::PostMsg({ msgTo(MessageType::MEETING_OK), seeker::time::currentTime()});
      }
      else hi::PostMsg({ msgTo(MessageType::MEETING_OK), info.timePoint() });
    }
    else if (info.cmeth() == "INVITE_SHARE") {
      I_LOG("open share success");
    }
    else if (info.cmeth() == "INFO" && info.signal() == "31") {
      callback_->OnInfoSuccess();
    }
  }

  void SignlingInteractionSystem::OnTrying(const SignInfo& info) {
    signalState = State::TRYING;
  }

  void SignlingInteractionSystem::OnRinging(const SignInfo& info) {
    signalState = State::RINGING;
    callback_->OnRinging();
  }

  void SignlingInteractionSystem::OnUnauthorized(const SignInfo& info) {
    if (info.cmeth() == "REGISTER") {
      W_LOG("login {}:{} -> {} failed", serverInfo.serverIp_, serverInfo.serverPort_, userInfo.id_);
      serverInfo.clear();
    }
  }


  //private
  bool SignlingInteractionSystem::connect(const ServerInfo& info) {
    try {
      if (client) {
        logout();
        I_LOG("[SignlingInteractionSystem::connectServer] login out! new server is {}:{}",
          info.serverIp_, info.serverPort_);
      }
      I_LOG("signling link 1");
      auto connectionProvider =
        oatpp::network::tcp::client::ConnectionProvider::createShared({ info.serverIp_, info.serverPort_ });
      I_LOG("signling link 2");
      auto connector = oatpp::websocket::Connector::createShared(connectionProvider);
      I_LOG("signling link 3");
      auto connection = connector->connect("/connectWS");
      I_LOG("signling link 4");
      client = oatpp::websocket::WebSocket::createShared(connection, true);
      I_LOG("signling link 5");
      client->setListener(listener);
      I_LOG("signling link 6");
      signalState = State::CONNECT_ON;
    }
    catch (std::exception& ex) {
      E_LOG("[SignlingInteractionSystem::connectServer] connect sever failed:{}", ex.what());
      return false;
    }
    catch (...) {
      E_LOG("[SignlingInteractionSystem::connectServer] connect sever failed");
      return false;
    }
    return true;
  }

  bool SignlingInteractionSystem::ToSignaling(const SignInfo& info) {
    oatpp::String js = oatpp::String(info.js.dump());
    I_LOG("signling message send:{}", info.js.dump(4));
    return client->sendOneFrame(true, oatpp::websocket::Frame::OPCODE_TEXT, js);
  }
}
