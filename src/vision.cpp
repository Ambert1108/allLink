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
       
  void VisionCentralContoller::switchStreamScreen() {

  }

  void VisionCentralContoller::switchStartScreen() {

  }

  void VisionCentralContoller::startLocalRenderer(webrtc::VideoTrackInterface* local_video) {
  
  }
  
  void VisionCentralContoller::stopLocalRenderer() {
  
  }
  
  void VisionCentralContoller::startRemoteRenderer(webrtc::VideoTrackInterface* remote_video) {
  
  }
  
  void VisionCentralContoller::stopRemoteRenderer() {
  
  }

  void VisionCentralContoller::pollEvent() {
    /* 处理窗口事件 */
    wnd->eventProcess();
    loginWnd->eventProcess();
    enterWnd->eventProcess();
  }

  void VisionCentralContoller::update() {
    /* 读取并处理自定义消息事件 */
    if (!hi::GetMsg(msg)) return;
    switch (msg.id) {
    case msgTo(MessageType::CREATE_MEETING): {
      if (type_ == VisionType::LOGOUT) break;
      //收到开始窗口请求创建会议交互，显示进入会议窗口
      if (!enterWnd->OnEnter()) break;
      std::shared_ptr<EnterScreen> point = std::dynamic_pointer_cast<EnterScreen>(enterWnd);
      if (!point) break;
      point->setType(EnterScreen::EnterType::CREATE);
      break;
    }
    case msgTo(MessageType::JOIN_MEETING): {
      if (type_ == VisionType::LOGOUT) break;
      //收到开始窗口请求加入会议交互，显示进入会议窗口
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
      callback_->StartLogin(LinkInfo(loginInfo.at(0)), {loginInfo.at(1), loginInfo.at(2)});
      //TODO:如果登录成功，调用OnExit方法关闭登录窗口
      //TODO:如果登录失败，调用setError方法告知用户，让用户重新登录

      //这里先假设
      break;
    }
    case msgTo(MessageType::IS_ENTER): {
      if (type_ == VisionType::LOGOUT) break;
      std::vector<std::string> meetingInfo = std::any_cast<std::vector<std::string>>(msg.data);
      if (meetingInfo.empty()) {
        break;
      }
      I_LOG("[debug] meeting id is {}", meetingInfo.at(0));
      
      //调用中控器的回调接口进行通话连接
      callback_->ConnectToPeer(meetingInfo.at(0));
      break;
    }
    case msgTo(MessageType::LOGIN_SUCCESS): {
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
    default:
      break;
    }
  }

  void VisionCentralContoller::render() {
    wnd->show();
    loginWnd->show();
    enterWnd->show();
  }
}