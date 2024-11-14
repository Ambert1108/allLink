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
    wnd = std::make_shared<LoginScreen>(sf::VideoMode(640, 480), "AllLink", icon, sf::Style::Default);
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
    /* 处理自定义消息事件 */
  }

  void VisionCentralContoller::render() {
    wnd->show();
  }
}