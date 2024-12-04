#include "vision.h"

#include <iostream>
#include <cctype>
#include <math.h>
#include <conio.h>

#include "api/video/i420_buffer.h"
#include "rtc_base/arraysize.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "third_party/libyuv/include/libyuv/convert_argb.h"
#include "libyuv.h"

namespace alllink {
  VisionCentralContoller::VisionCentralContoller() 
    : callback_(nullptr) {
    sf::Image icon;
    icon.loadFromFile(iconFile);
    float wr = static_cast<float>(sf::VideoMode::getDesktopMode().width) / 1920;
    float hr = static_cast<float>(sf::VideoMode::getDesktopMode().height) / 1080;
    wnd = std::make_shared<StartScreen>(sf::VideoMode(640 * wr, 480 * hr), "AllLink", icon, CustomScreen::Style::Minisize);
    wnd->init();
    wnd->OnEnter();
    loginWnd = std::make_shared<LoginScreen>(sf::VideoMode(478 * wr, 353 * hr), "AllLink", icon, CustomScreen::Style::Minisize);
    loginWnd->init();
    enterWnd = std::make_shared<EnterScreen>(sf::VideoMode(356 * wr, 562 * hr), "AllLink", icon, CustomScreen::Style::Minisize);
    enterWnd->init();
    streamWnd = std::make_shared<StreamScreen>(sf::VideoMode(1920 * wr, 1080 * hr), "AllLink", icon);
    streamWnd->init();
  }

  VisionCentralContoller::~VisionCentralContoller() {

  }

  void VisionCentralContoller::registerObserver(VisionCnetralCallback* callback) { callback_ = callback; }
  
  void VisionCentralContoller::run() {
    while (wnd->isOpen()) {
      pollEvent();
      update();
      render();
    }
  }

  void VisionCentralContoller::startLocalRenderer(webrtc::VideoTrackInterface* local_video) {
    std::shared_ptr<StreamScreen> point = std::dynamic_pointer_cast<StreamScreen>(streamWnd);
    if (!point) return;
    point->startLocalRenderer(local_video);
  }
  
  void VisionCentralContoller::stopLocalRenderer() {
    std::shared_ptr<StreamScreen> point = std::dynamic_pointer_cast<StreamScreen>(streamWnd);
    if (!point) return;
    point->stopLocalRenderer();
  }
  
  void VisionCentralContoller::startRemoteRenderer(webrtc::VideoTrackInterface* remote_video) {
    std::shared_ptr<StreamScreen> point = std::dynamic_pointer_cast<StreamScreen>(streamWnd);
    if (!point) return;
    point->startRemoteRenderer(remote_video);
  }
  
  void VisionCentralContoller::stopRemoteRenderer() {
    std::shared_ptr<StreamScreen> point = std::dynamic_pointer_cast<StreamScreen>(streamWnd);
    if (!point) return;
    point->stopRemoteRenderer();
  }

  void VisionCentralContoller::pollEvent() {
    /* 处理窗口事件 */
    wnd->eventProcess();
    loginWnd->eventProcess();
    enterWnd->eventProcess();
    streamWnd->eventProcess();
  }

  void VisionCentralContoller::update() {
    /* 读取并处理自定义消息事件 */
    if (!hi::GetMsg(msg)) return;
    try {
      switch (msg.id) {
        case msgTo(MessageType::CREATE_MEETING): {
          if (type_ == VisionType::LOGOUT) break;
          //收到开始窗口请求创建会议交互，显示连接窗口
          if (!enterWnd->OnEnter()) break;
          std::shared_ptr<EnterScreen> point = std::dynamic_pointer_cast<EnterScreen>(enterWnd);
          if (!point) break;
          point->setType(EnterScreen::EnterType::CREATE);
          break;
        }
        case msgTo(MessageType::JOIN_MEETING): {
          if (type_ == VisionType::LOGOUT) break;
          //收到开始窗口请求加入会议交互，显示连接窗口
          if (!enterWnd->OnEnter()) break;
          std::shared_ptr<EnterScreen> point = std::dynamic_pointer_cast<EnterScreen>(enterWnd);
          if (!point) break;
          point->setType(EnterScreen::EnterType::JOIN);
          break;
        }
        case msgTo(MessageType::START_LOGIN):
          // 收到开始窗口请求登录交互，显示登录窗口
          loginWnd->OnEnter();
          break;
        case msgTo(MessageType::START_LOGOUT):
          break;
        case msgTo(MessageType::SETTING):
          break;
        case msgTo(MessageType::IS_LOGIN): {
          // 收到登录窗口用户输入交互

          // 取出消息中的登录信息
          std::vector<std::string> loginInfo = std::any_cast<std::vector<std::string>>(msg.data);
          if (loginInfo.empty() || loginInfo.size() < 3) {
            W_LOG("[VisionCentralContoller::update] login info is empty");
            break;
          }
          I_LOG("[debug] server addr:{}, useId:{}, usePwd:{}", loginInfo.at(0), loginInfo.at(1), loginInfo.at(2));

          //调用中控器的回调接口进行具体的登录操作
          callback_->StartLogin(ServerInfo(loginInfo.at(0)), {loginInfo.at(1), loginInfo.at(2)});
          break;
        }
        case msgTo(MessageType::IS_ENTER): {
          if (type_ == VisionType::LOGOUT) break;
          // 收到连接窗口连接消息
          std::vector<std::string> meetingInfo = std::any_cast<std::vector<std::string>>(msg.data);
          if (meetingInfo.empty()) {
            break;
          }
          I_LOG("[debug] meeting id is {}", meetingInfo.at(0));
      
          // 调用中控器的回调接口进行通话连接
          callback_->ConnectToPeer(meetingInfo.at(0));

          // 假设连接成功，隐藏连接窗口及开始窗口，显示会议窗口
          //I_LOG("[test] link success");
          //enterWnd->OnExit();
          //wnd->OnExit();
          //streamWnd->OnEnter();
          break;
        }
        case msgTo(MessageType::LOGIN_SUCCESS): {
          //TODO:如果登录失败，调用setError方法告知用户，让用户重新登录
          //收到信令回复登录成功，关闭登录窗口
          loginWnd->OnExit();
          type_ = VisionType::LOGIN;
          // 将用户名提供给开始窗口
          std::shared_ptr<StartScreen> point = std::dynamic_pointer_cast<StartScreen>(wnd);
          if (!point) break;
          std::string userId = std::any_cast<std::string>(msg.data);
          point->setUseId(userId);
          break;
        }
        case msgTo(MessageType::MEETING_END): {
          // 收到会议窗口被关闭
          I_LOG("actively hand up, close stream screen");
          // 隐藏会议窗口
          streamWnd->OnExit();
          // 显示开始窗口
          wnd->OnEnter();
          // 调用DisconnectFromCurrentPeer方法通知中控器断开连接
          callback_->DisconnectFromCurrentPeer();
          break;
        }
        case msgTo(MessageType::DISCONNECT_PEER):
          I_LOG("passive hand up, close stream screen");
          // 隐藏会议窗口
          streamWnd->OnExit();
          // 显示开始窗口
          wnd->OnEnter();
          // 调用DisconnectFromCurrentPeer方法通知中控器断开连接
          callback_->CustomMessageCallback(msg);
          break;
        case msgTo(MessageType::SET_REMOTE_DESC):
        case msgTo(MessageType::SEND_PROCESS_TO_JANUS):
        case msgTo(MessageType::SEND_JSEP_SDP_TO_PEER):
        case msgTo(MessageType::SEND_SDP_TO_PEER): 
        case msgTo(MessageType::SEND_ICE_COMPLETE_TO_PEER):
        case msgTo(MessageType::SEND_ICE_TO_PEER):
          // 中控器需要发送sdp/ice消息
          I_LOG("[test] send {} to peer", msg.id);
          // 通知中控器处理消息数据
          callback_->CustomMessageCallback(msg);
          break;
        case msgTo(MessageType::SEND_MSG_FAILED): {
          // 中控器提示offer sdp创建失败
          break;
        }
        case msgTo(MessageType::ADD_TRACK): {
          // 中控器收到创建轨道回调，通话建立成功
        
          // 连接成功，隐藏连接窗口及开始窗口，显示会议窗口
          auto* track = std::any_cast<webrtc::MediaStreamTrackInterface*>(msg.data);
          if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
            I_LOG("add video track");
            enterWnd->OnExit();
            wnd->OnExit();
            streamWnd->OnEnter();
            std::shared_ptr<StreamScreen> point = std::dynamic_pointer_cast<StreamScreen>(streamWnd);
            if (!point) return;
            auto* video_track = static_cast<webrtc::VideoTrackInterface*>(track);
            point->startRemoteRenderer(video_track);
            I_LOG("[test] add track, link success");
          }
          else I_LOG("add audio track");
          track->Release();
        
          break;
        }
        case msgTo(MessageType::REMOVE_TRACK): {
          // 中控器收到移除轨道回调，通话结束
          webrtc::MediaStreamTrackInterface* track = std::any_cast<webrtc::MediaStreamTrackInterface*>(msg.data);
          if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
            I_LOG("remove video track");
          }
          else {
            I_LOG("remove audio track");
          }
          track->Release();
          break;
        }
        default:
          break;
      }
    }
    catch (std::exception& ex) {
      E_LOG("[VisionCentralContoller::update] catch exception:{}", ex.what());
    }
  }

  void VisionCentralContoller::render() {
    wnd->show();
    loginWnd->show();
    enterWnd->show();
    streamWnd->show();
  }
}