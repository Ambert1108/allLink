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
  closeMic.setImageColor(sf::Color::Black);

  openMic.init(80, 50, 20, 1020);
  openMic.setTexture(openMicFile);
  openMic.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  openMic.setImageSize(40, 40);
  openMic.setImageColor(sf::Color(74, 224, 84));

  closeCam.init(80, 50, 140, 1020);
  closeCam.setTexture(closeCamFile);
  closeCam.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  closeCam.setImageSize(40, 40);
  closeCam.setImageColor(sf::Color::Black);

  openCam.init(80, 50, 140, 1020);
  openCam.setTexture(openCamFile);
  openCam.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  openCam.setImageSize(40, 40);
  openCam.setImageColor(sf::Color(117, 188, 255));

  closeShare.init(80, 50, 280, 1020);
  closeShare.setTexture(closeShareFile);
  closeShare.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  closeShare.setImageSize(40, 40);
  closeShare.setImageColor(sf::Color::Black);

  openShare.init(80, 50, 280, 1020);
  openShare.setTexture(openShareFile);
  openShare.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  openShare.setImageSize(40, 40);
  openShare.setImageColor(sf::Color(242, 80, 125));

  HorizonGraphicTextsModule meetingTime(false);
  meetingTime.init(240, 16, 6, 12);
  meetingTime.setSource(18, font2File, timeFile);
  meetingTime.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  meetingTime.setText(L"会议时长 0:0:0", sf::Color(0, 0, 0));
  meetingTime.setImageSize(24, 24);
  meetingTime.setImageColor(sf::Color(117, 188, 255));
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

  TextFillRectangle leaveMeeting(sf::Color::Red, 3);
  leaveMeeting.init(190, 50, 1686, 1015, 8.0);
  leaveMeeting.setColor(sf::Color::White, sf::Color::Red, sf::Color(191, 23, 23));
  leaveMeeting.setText(font2File, L"离开会议", sf::Color::Black, sf::Color::White);

  BaseText meetingDescribe;
  meetingDescribe.init(font2File);
  meetingDescribe.setCharacterSize(18);
  meetingDescribe.setString(L"会议号 000-512");
  meetingDescribe.setFillColor(sf::Color::Black);
  meetingDescribe.setPosition((1920 - meetingDescribe.getGlobalBounds().width) / 2, 11);

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

  SeekBarModule seekbar;
  seekbar.init(sf::Vector2f(210, 9), 13, sf::Vector2f(35, 622), sf::Color::White, sf::Color(68, 118, 235));
  seekbar.setText(fontFile, 0, 21, sf::Color::Black);

  VariableStateVertxModule micArrow;
  micArrow.set(14, 50, 101, 1020);
  micArrow.setVer({
    sf::Vertex(sf::Vector2f(101, 1050), sf::Color::Black),
    sf::Vertex(sf::Vector2f(101 + 7, 1040), sf::Color::Black),
    sf::Vertex(sf::Vector2f(101 + 7, 1040), sf::Color::Black),
    sf::Vertex(sf::Vector2f(101 + 14, 1050), sf::Color::Black) });
  micArrow.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));

  VariableStateVertxModule camArrow;
  camArrow.set(14, 50, 221, 1020);
  camArrow.setVer({
    sf::Vertex(sf::Vector2f(221, 1050), sf::Color::Black),
    sf::Vertex(sf::Vector2f(221 + 7, 1040), sf::Color::Black),
    sf::Vertex(sf::Vector2f(221 + 7, 1040), sf::Color::Black),
    sf::Vertex(sf::Vector2f(221 + 14, 1050), sf::Color::Black) });
  camArrow.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));

  VariableStateVertxModule shareArrow;
  shareArrow.set(14, 50, 361, 1020);
  shareArrow.setVer({
    sf::Vertex(sf::Vector2f(361, 1050), sf::Color::Black),
    sf::Vertex(sf::Vector2f(361 + 7, 1040), sf::Color::Black),
    sf::Vertex(sf::Vector2f(361 + 7, 1040), sf::Color::Black),
    sf::Vertex(sf::Vector2f(361 + 14, 1050), sf::Color::Black) });
  shareArrow.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));

  DropListModule micDrop;
  micDrop.init(280, 150, 15, 650, 45, fontFile);
  micDrop.addLabel(L"麦克风阵列", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  micDrop.addLabel(L"logic microphone", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  micDrop.addLabel(L"麦克风", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  micDrop.setShow(true);

  DropListModule camDrop;
  camDrop.init(280, 150, 60, 650, 45, fontFile);
  camDrop.addLabel(L"Logic C270 HD Webcam", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  camDrop.addLabel(L"USB camera", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  camDrop.setShow(true);

  DropListModule shareDrop;
  shareDrop.init(280, 150, 200, 650, 45, fontFile);
  shareDrop.addLabel(L"1", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  shareDrop.addLabel(L"2", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  shareDrop.setShow(true);

  sf::Vector2f micPos{ 5, 600 };
  sf::Vector2f camPos{ 50, 600 };
  sf::Vector2f sharePos{ 190, 600 };

  VariableStateFillModule background(sf::Color::Transparent, 2);
  background.setSize(sf::Vector2f(300, 400), 5);
  background.setPosition(micPos.x, micPos.y);
  background.setFillColor(sf::Color(220, 220, 220));

  bool settingPop = false;

  wnd->setSize(sf::Vector2u(1280, 720));
  int64_t timePoint = seeker::time::currentTime();

  bool micArrowClick = false;
  bool camArrowClick = false;
  bool shareArrowClick = false;
  while (wnd->isOpen()) {
    sf::Vector2f mousePosView;
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
        if (leaveMeeting.onClick(event, mousePosView, wnd)) {
          I_LOG("离开会议");
        }
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

      mousePosView = wnd->mapPixelToCoords(mousePosWin);
      if (micArrow.onClick(event, mousePosView, wnd)) {
        micArrowClick = true;
        camArrowClick = false;
        shareArrowClick = false;
        settingPop = true;
        background.setPosition(micPos.x, micPos.y);
        I_LOG("mic arrow click");
      }
      else if (camArrow.onClick(event, mousePosView, wnd)) {
        camArrowClick = true;
        micArrowClick = false;
        shareArrowClick = false;
        settingPop = true;
        background.setPosition(camPos.x, camPos.y);
        I_LOG("cam arrow click");
      }
      else if (shareArrow.onClick(event, mousePosView, wnd)) {
        shareArrowClick = true;
        micArrowClick = false;
        camArrowClick = false;
        settingPop = true;
        background.setPosition(sharePos.x, sharePos.y);
        I_LOG("share arrow click");
      }
      if (background.getGlobalBounds().contains(mousePosView)) {
        seekbar.eventProcess(event, wnd);
        if (micArrowClick) {
          if (micDrop.eventProcess(event, mousePosView, wnd, false)) {
            I_LOG("mic labal is {}", WstrConv.to_bytes(micDrop.getSelectedLabel()));
          }
        }
        if (camArrowClick) {
          if(camDrop.eventProcess(event, mousePosView, wnd, false)) {
            I_LOG("cam labal is {}", WstrConv.to_bytes(camDrop.getSelectedLabel()));
          }
        }
        if (shareArrowClick) {
          if (shareDrop.eventProcess(event, mousePosView, wnd, false)) {
            I_LOG("share labal is {}", WstrConv.to_bytes(shareDrop.getSelectedLabel()));
          }
        }
      }
      else {
        if (event.type == sf::Event::MouseButtonReleased
          && event.mouseButton.button == sf::Mouse::Left
          && !micArrowClick && !camArrowClick && !shareArrowClick) {
          if (settingPop) settingPop = false;
        }
        if (isFull && settingPop) settingPop = false;
      }
    }
    std::wstring time = L"会议时长 " + 
      converter.from_bytes(parseTime(seeker::time::currentTime() - timePoint));
    meetingTime.setText(time, sf::Color(0, 0, 0));

    if (seekbar.update(wnd)) {
      I_LOG("horseekbar data is {}", seekbar.data());
    }
    wnd->clear(sf::Color(240, 240, 240));
    if (!isFull) {
      wnd->draw(top);
      wnd->draw(bottom);
      leaveMeeting.render(wnd);
      if (micState) openMic.render(wnd);
      else closeMic.render(wnd);

      if (camState) openCam.render(wnd);
      else closeCam.render(wnd);

      if (shareState) openShare.render(wnd);
      else closeShare.render(wnd);
      meetingTime.render(wnd);
      wnd->draw(meetingDescribe);
      if (settingPop) {
        wnd->draw(background);
        if (micArrowClick) {
          seekbar.render(wnd);
          micDrop.render(wnd);
        }
        else if (camArrowClick) {
          camDrop.render(wnd);
        }
        else if (shareArrowClick) {
          shareDrop.render(wnd);
        }
      }
      micArrow.render(wnd);
      camArrow.render(wnd);
      shareArrow.render(wnd);
    }
    wnd->display();
  }

  return 0;
}