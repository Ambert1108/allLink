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
    wnd = std::make_shared<LoginScreen>(sf::VideoMode(640 * wr, 480 * hr), "AllLink", icon, sf::Style::Titlebar | sf::Style::Close);
    I_LOG("init start");
    wnd->init();
    I_LOG("init finish");
    wnd->OnEnter();
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
  }

  void VisionCentralContoller::update() {
    /* 读取并处理自定义消息事件 */
    if (!hi::GetMsg(msg)) return;
    switch (msg.id) {
    case msgTo(MessageType::START_LOGIN):
      break;

    default:
      break;
    }
  }

  void VisionCentralContoller::render() {
    wnd->show();
  }
}