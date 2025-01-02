#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "component/module.h"
#include "component/widget.h"
#include "seeker/common.h"

#include <locale>
#include <codecvt>

inline std::string parseTime(int64_t timestamp) {
  timestamp *= 0.001;
  int64_t hour = timestamp / 3600;
  int64_t min = timestamp / 60 - hour * (int64_t)60;
  int64_t sec = timestamp - hour * (int64_t)3600 - min * (int64_t)60;
  std::string s = std::to_string(hour) + ":" + std::to_string(min) + ":" + std::to_string(sec) + "";
  return s;
}

int main() {
  using namespace alllink;
  I_LOG("time:{}", seeker::time::toString(seeker::time::currentTime()));
  sf::RenderWindow* wnd = new sf::RenderWindow(
    sf::VideoMode(1920, 1080),
    "stream",
    sf::Style::Default);
  wnd->setFramerateLimit(60);
  sf::Event event{};
  std::string fontFile = "./resources/fonts/fzch.ttf";
  std::string font2File = "./resources/fonts/msyh.ttc";
  std::string closeMicFile = "./resources/comp/meeting/close_mic.png";
  std::string openMicFile = "./resources/comp/meeting/open_mic.png";
  std::string closeCamFile = "./resources/comp/meeting/close_camera.png";
  std::string openCamFile = "./resources/comp/meeting/open_camera.png";
  std::string closeShareFile = "./resources/comp/meeting/close_Share.png";
  std::string openShareFile = "./resources/comp/meeting/open_Share.png";
  std::string timeFile = "./resources/comp/meeting/16/meeting_time.png";

  VariableStateGraphicModule closeMic, openMic, closeCam, openCam, closeShare, openShare;
  closeMic.init(80, 50, 20, 1020);
  closeMic.setTexture(closeMicFile);
  closeMic.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  closeMic.setImageSize(40, 40);

  openMic.init(80, 50, 20, 1020);
  openMic.setTexture(openMicFile);
  openMic.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  openMic.setImageSize(40, 40);

  closeCam.init(80, 50, 140, 1020);
  closeCam.setTexture(closeCamFile);
  closeCam.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  closeCam.setImageSize(40, 40);

  openCam.init(80, 50, 140, 1020);
  openCam.setTexture(openCamFile);
  openCam.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  openCam.setImageSize(40, 40);

  closeShare.init(80, 50, 280, 1020);
  closeShare.setTexture(closeShareFile);
  closeShare.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  closeShare.setImageSize(40, 40);

  openShare.init(80, 50, 280, 1020);
  openShare.setTexture(openShareFile);
  openShare.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  openShare.setImageSize(40, 40);

  HorizonGraphicTextsModule meetingTime;
  meetingTime.init(240, 16, 6, 12);
  meetingTime.setSource(18, font2File, timeFile);
  meetingTime.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  meetingTime.setText(L"会议时长 0:0:0", sf::Color(0, 0, 0));
  meetingTime.setImageSize(24, 24);
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

  bool micState = false;
  bool camState = false;
  bool shareState = false;
  bool isFull = true;

  sf::RectangleShape bottom;
  bottom.setPosition(0, 1000);
  bottom.setSize(sf::Vector2f(1920, 80));
  bottom.setFillColor(sf::Color(255, 255, 255));

  sf::RectangleShape top;
  top.setPosition(0, 0);
  top.setSize(sf::Vector2f(1920, 40));
  top.setFillColor(sf::Color(255, 255, 255));

  wnd->setSize(sf::Vector2u(1280, 720));
  int64_t timePoint = seeker::time::currentTime();
  while (wnd->isOpen()) {
    while (wnd->pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        wnd->close();
        break;
      }
      sf::Vector2i mousePosWin = sf::Mouse::getPosition(*wnd);
      // 检查鼠标是否在窗口内
      if (mousePosWin.x >= 0 && mousePosWin.x < wnd->getSize().x &&
        mousePosWin.y >= 0 && mousePosWin.y < wnd->getSize().y) {
        sf::Vector2f mousePosView = wnd->mapPixelToCoords(mousePosWin);
        if (!micState) {
          if (closeMic.onClick(event, mousePosView, wnd)) {
            I_LOG("开启麦克风");
            micState = !micState;
          }
        }
        else {
          if (openMic.onClick(event, mousePosView, wnd)) {
            I_LOG("关闭麦克风");
            micState = !micState;
          }
        }
        if (!camState) {
          if (closeCam.onClick(event, mousePosView, wnd)) {
            I_LOG("开启摄像头");
            camState = !camState;
          }
        }
        else {
          if (openCam.onClick(event, mousePosView, wnd)) {
            I_LOG("关闭摄像头");
            camState = !camState;
          }
        }
        if (!shareState) {
          if (closeShare.onClick(event, mousePosView, wnd)) {
            I_LOG("开启屏幕共享");
            shareState = !shareState;
          }
        }
        else {
          if (openShare.onClick(event, mousePosView, wnd)) {
            I_LOG("关闭屏幕共享");
            shareState = !shareState;
          }
        }
        isFull = false;
      }
      else isFull = true;
    }
    std::wstring time = L"会议时长 " + 
      converter.from_bytes(parseTime(seeker::time::currentTime() - timePoint));
    meetingTime.setText(time, sf::Color(0, 0, 0));
    wnd->clear(sf::Color(240, 240, 240));
    if (!isFull) {
      wnd->draw(top);
      wnd->draw(bottom);
      if (micState) openMic.render(wnd);
      else closeMic.render(wnd);

      if (camState) openCam.render(wnd);
      else closeCam.render(wnd);

      if (shareState) openShare.render(wnd);
      else closeShare.render(wnd);
      meetingTime.render(wnd);
    }
    wnd->display();
  }

  return 0;
}