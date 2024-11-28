#pragma once

#include <map>
#include <memory>
#include <string>
#include <mutex>
#include <regex>

#include "seeker/logger.h"
#include "seeker/loggerApi.h"

#include "signling.h"
#include "januslistener.h"
#include "message.h"
#include "utils/InvokeTimer.hpp"

namespace alllink {
  struct SessionInfo {
    int64_t seeionId_;
    int64_t handleId_;

    SessionInfo() = default;

    bool operator==(const SessionInfo& other) const {
      return seeionId_ == other.seeionId_ && handleId_ == other.handleId_;
    }

    SessionInfo& operator=(const SessionInfo& other) {
      if (this != &other) {
        seeionId_ = other.seeionId_;
        handleId_ = other.handleId_;
      }
      return *this;
    }
  };

  struct JanusInteractionObserver {
    /*通知 中控器 接收到信令转发的其他客户端发送过来的信息*/
    virtual void OnGenerated(const std::string& sdp, const std::string& type) = 0;

    virtual void OnProcessed(const std::string& sdp, const std::string& type) = 0;

  protected:
    virtual ~JanusInteractionObserver() {}
  };

  class JanusInteractionSystem : public JanusListenObserver {
  public:
    enum State {
      NONE,

      // 未创建会话或正在向Janus创建会话并等待回复
      SESSIONING = 1,

      // 正在向Janus创建句柄，等待回复
      ATTACHING,

      // 目前处于闲置状态
      WAIT,

      // 正在向Janus发送generate请求，等待回复
      GENERATEING,

      // 正在向Janus发送process请求，等待回复
      PROCESSING
    };

    JanusInteractionSystem();
    ~JanusInteractionSystem();

    bool isConnected() const;

    void registerObserver(JanusInteractionObserver* callback);

    bool connectServer(const ServerInfo& info);

    bool sendTrckileToJanus(const std::string& ice);

    bool sendTrckileCompleteToJanus();

    bool sendGenerateToJanus(const std::string& jsepSdp, const std::string& type);

    bool sendProcessToJanus(const std::string& sdp, const std::string& type);


  protected:
    //
    // JanusListenObserver implementation.
    //
    void OnSuccess(const JanusReponse& resp) override;
    void OnAck(const JanusReponse& resp) override;
    void OnEvent(const JanusReponse& resp) override;

  private:
    JanusInteractionObserver* callback_;
    static constexpr const char* TAG = "JanusClient";
    std::shared_ptr<oatpp::websocket::WebSocket> client;
    std::shared_ptr<JanusListener> listener;
    aom::InvokeTimerPtr listenBody;
    aom::InvokeTimerPtr keepBody;
    ServerInfo serverInfo;
    SessionInfo sessionInfo;
    State state{ SESSIONING };
    int64_t lastBeatPoint = 0;
    int64_t HeartbeatInterval = 1000; //心跳间隔
    int64_t cseq_ = 0;
  };
}