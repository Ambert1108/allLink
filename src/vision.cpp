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
    loginWnd = std::make_unique<LoginScreen>(sf::VideoMode(478 * wr, 353 * hr), "AllLink", icon, CustomScreen::Style::Minisize);
    loginWnd->init();
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
       
  void VisionCentralContoller::switchNextScreen() {

  }

  void VisionCentralContoller::switchLastScreen() {

  }

  void VisionCentralContoller::startLocalRenderer(webrtc::VideoTrackInterface* local_video) {
  
  }
  
  void VisionCentralContoller::stopLocalRenderer() {
  
  }
  
  void VisionCentralContoller::startRemoteRenderer(webrtc::VideoTrackInterface* remote_video) {
  
  }
  
  void VisionCentralContoller::stopRemoteRenderer() {
  
  }

  void VisionCentralContoller::sendCustomMessage(int msg_id, void* data) {

  }

  void VisionCentralContoller::pollEvent() {
    /* 处理窗口事件 */
    wnd->eventProcess();
    loginWnd->eventProcess();
  }

  void VisionCentralContoller::update() {
    /* 读取并处理自定义消息事件 */
    if (!hi::GetMsg(msg)) return;
    switch (msg.id) {
    case msgTo(MessageType::CREATE_MEETING):
      break;
    case msgTo(MessageType::JOIN_MEETING):
      break;
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
      std::shared_ptr<StartScreen> point = std::dynamic_pointer_cast<StartScreen>(wnd);
      if (!point) break;

      // 取出消息中的登录信息
      std::vector<std::string> loginInfo = std::any_cast<std::vector<std::string>>(msg.data);
      if (loginInfo.empty() || loginInfo.size() < 3) {
        W_LOG("[VisionCentralContoller::update] login info is empty");
        break;
      }
      I_LOG("[debug] server addr:{}, useId:{}, usePwd:{}", loginInfo.at(0), loginInfo.at(1), loginInfo.at(2));
      // 将用户名提供给开始窗口
      point->setUseId(loginInfo.at(1));

      //调用中控器的回调接口进行具体的登录操作
      callback_->StartLogin(loginInfo.at(0), {loginInfo.at(1), loginInfo.at(2)});
      //TODO:如果登录成功，调用OnExit方法关闭登录窗口
      //TODO:如果登录失败，调用setError方法告知用户，让用户重新登录

      //这里先假设登录成功
      loginWnd->OnExit();
      break;
    }
    default:
      break;
    }
  }

  void VisionCentralContoller::render() {
    wnd->show();
    loginWnd->show();
  }
}