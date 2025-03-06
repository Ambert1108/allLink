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

  VariableStateGraphicRoundModule closeMic, openMic, closeCam, openCam, closeShare, openShare;
  closeMic.init(80, 50, 20, 1020, 6.f);
  closeMic.setTexture(closeMicFile);
  closeMic.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  closeMic.setImageSize(40, 40);
  closeMic.setImageColor(sf::Color::Black);

  openMic.init(80, 50, 20, 1020, 6.f);
  openMic.setTexture(openMicFile);
  openMic.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  openMic.setImageSize(40, 40);
  openMic.setImageColor(sf::Color(74, 224, 84));

  closeCam.init(80, 50, 140, 1020, 6.f);
  closeCam.setTexture(closeCamFile);
  closeCam.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  closeCam.setImageSize(40, 40);
  closeCam.setImageColor(sf::Color::Black);

  openCam.init(80, 50, 140, 1020, 6.f);
  openCam.setTexture(openCamFile);
  openCam.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  openCam.setImageSize(40, 40);
  openCam.setImageColor(sf::Color(117, 188, 255));

  closeShare.init(80, 50, 280, 1020, 6.f);
  closeShare.setTexture(closeShareFile);
  closeShare.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  closeShare.setImageSize(40, 40);
  closeShare.setImageColor(sf::Color::Black);

  openShare.init(80, 50, 280, 1020, 6.f);
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

  TextRoundRectangle leaveMeeting(sf::Color::Red, 3);
  leaveMeeting.init(190, 50, 1686, 1015, 8.0);
  leaveMeeting.setColor(sf::Color::White, sf::Color::Red, sf::Color(191, 23, 23));
  leaveMeeting.setText(font2File, L"离开会议", sf::Color::Black, sf::Color::White);

  ClickTextRectangle meetingDescribe;
  meetingDescribe.init(150, 40, ((1920 - meetingDescribe.getGlobalBounds().width) / 2), 0);
  meetingDescribe.setText(font2File, L"333-161", sf::Color::Black);
  meetingDescribe.setColor(sf::Color(200, 200, 200, 0), sf::Color(230, 230, 230, 100), sf::Color(215, 215, 215, 100));

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

  VariableStateVertxRoundModule micArrow;
  micArrow.set(14, 50, 101, 1020, 5.f);
  micArrow.setVer({
    sf::Vertex(sf::Vector2f(101, 1050), sf::Color::Black),
    sf::Vertex(sf::Vector2f(101 + 7, 1040), sf::Color::Black),
    sf::Vertex(sf::Vector2f(101 + 7, 1040), sf::Color::Black),
    sf::Vertex(sf::Vector2f(101 + 14, 1050), sf::Color::Black) });
  micArrow.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));

  VariableStateVertxRoundModule camArrow;
  camArrow.set(14, 50, 221, 1020, 5.f);
  camArrow.setVer({
    sf::Vertex(sf::Vector2f(221, 1050), sf::Color::Black),
    sf::Vertex(sf::Vector2f(221 + 7, 1040), sf::Color::Black),
    sf::Vertex(sf::Vector2f(221 + 7, 1040), sf::Color::Black),
    sf::Vertex(sf::Vector2f(221 + 14, 1050), sf::Color::Black) });
  camArrow.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));

  VariableStateVertxRoundModule shareArrow;
  shareArrow.set(14, 50, 361, 1020, 5.f);
  shareArrow.setVer({
    sf::Vertex(sf::Vector2f(361, 1050), sf::Color::Black),
    sf::Vertex(sf::Vector2f(361 + 7, 1040), sf::Color::Black),
    sf::Vertex(sf::Vector2f(361 + 7, 1040), sf::Color::Black),
    sf::Vertex(sf::Vector2f(361 + 14, 1050), sf::Color::Black) });
  shareArrow.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));


  sf::Vector2f micPos{ 5, 400 };
  sf::Vector2f camPos{ 50, 400 };
  sf::Vector2f sharePos{ 190, 400 };

  BaseText seekbarDescribe;
  seekbarDescribe.init(fontFile);
  seekbarDescribe.setCharacterSize(15);
  seekbarDescribe.setString(L"调节麦克风音量");
  seekbarDescribe.setPosition(micPos.x + 7, 415);
  seekbarDescribe.setFillColor(sf::Color::Black);

  SeekBarModule seekbar;
  seekbar.init(sf::Vector2f(210, 9), 13, sf::Vector2f(micPos.x + 30, 445), sf::Color::White, sf::Color(68, 118, 235));
  seekbar.setText(fontFile, 0, 21, sf::Color::Black);

  BaseText micDescribe;
  micDescribe.init(fontFile);
  micDescribe.setCharacterSize(15);
  micDescribe.setString(L"选择麦克风");
  micDescribe.setPosition(micPos.x + 7, 485);
  micDescribe.setFillColor(sf::Color::Black);

  DropListModule micDrop;
  micDrop.init(280, 150, micPos.x + 10, 510, 40, fontFile);
  micDrop.addLabel(L"麦克风阵列", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  micDrop.addLabel(L"logic microphone", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  micDrop.addLabel(L"麦克风", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  micDrop.addLabel(L"麦克风2", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  micDrop.addLabel(L"麦克风3", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  micDrop.setShow(true);

  BaseText spDescribe;
  spDescribe.init(fontFile);
  spDescribe.setCharacterSize(15);
  spDescribe.setString(L"选择扬声器");
  spDescribe.setPosition(micPos.x + 7, 755);
  spDescribe.setFillColor(sf::Color::Black);

  DropListModule spDrop;
  spDrop.init(280, 150, micPos.x + 10, 780, 40, fontFile);
  spDrop.addLabel(L"Arctis 5 Chat", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  spDrop.addLabel(L"EDIFIER M30", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  spDrop.addLabel(L"扬声器1", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  spDrop.addLabel(L"扬声器2", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  spDrop.addLabel(L"扬声器3", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  spDrop.setShow(true);

  BaseText camDescribe;
  camDescribe.init(fontFile);
  camDescribe.setCharacterSize(15);
  camDescribe.setString(L"选择摄像头");
  camDescribe.setPosition(camPos.x + 7, 630);
  camDescribe.setFillColor(sf::Color::Black);

  DropListModule camDrop;
  camDrop.init(280, 150, camPos.x + 10, 655, 40, fontFile);
  camDrop.addLabel(L"Logic C270 HD Webcam", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  camDrop.addLabel(L"USB camera", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  camDrop.setShow(true);

  BaseText screenDescribe;
  screenDescribe.init(fontFile);
  screenDescribe.setCharacterSize(15);
  screenDescribe.setString(L"选择屏幕");
  screenDescribe.setPosition(sharePos.x + 7, 410);
  screenDescribe.setFillColor(sf::Color::Black);

  DropListModule screenDrop;
  screenDrop.init(280, 150, sharePos.x + 10, 430, 40, fontFile);
  screenDrop.addLabel(L"1", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  screenDrop.addLabel(L"2", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  screenDrop.addLabel(L"3", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  screenDrop.setShow(true);

  BaseText windowDescribe;
  windowDescribe.init(fontFile);
  windowDescribe.setCharacterSize(15);
  windowDescribe.setString(L"选择窗口");
  windowDescribe.setPosition(sharePos.x + 7, 582);
  windowDescribe.setFillColor(sf::Color::Black);

  DropListModule windowDrop;
  windowDrop.init(280, 150, sharePos.x + 10, 602, 40, fontFile, 9);
  windowDrop.addLabel(L"窗口1", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  windowDrop.addLabel(L"窗口2", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  windowDrop.addLabel(L"窗口3", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  windowDrop.addLabel(L"窗口4", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  windowDrop.addLabel(L"窗口5", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  windowDrop.addLabel(L"窗口6", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  windowDrop.addLabel(L"窗口7", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  windowDrop.addLabel(L"窗口8", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  windowDrop.addLabel(L"窗口9", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(240, 240, 240));
  windowDrop.setShow(true);

  VariableStateRoundModule background(sf::Color::Transparent, 2);
  background.setSize(sf::Vector2f(300, 600));
  background.setCornerRadius(5);
  background.setPosition(sf::Vector2f(micPos.x, micPos.y));
  background.setFillColor(sf::Color(220, 220, 220));

  sf::RectangleShape sense;
  sense.setSize(sf::Vector2f(480, 80));
  sense.setPosition(sf::Vector2f(15, 1000));
  sense.setFillColor(sf::Color::Transparent);

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
        if (meetingDescribe.onClick(event, mousePosView, wnd)) {
          std::string id = WstrConv.to_bytes(meetingDescribe.getDescription());
          I_LOG("获取会议id:{}", id);
          toClipBoard(id);
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

      bool arrowClick = false;
      mousePosView = wnd->mapPixelToCoords(mousePosWin);
      if (micArrow.onClick(event, mousePosView, wnd)) {
        micArrowClick = true;
        camArrowClick = false;
        shareArrowClick = false;
        arrowClick = true;
        settingPop = true;
        background.setPosition(sf::Vector2f(micPos.x, micPos.y));
        I_LOG("mic arrow click");
      }
      else if (camArrow.onClick(event, mousePosView, wnd)) {
        camArrowClick = true;
        micArrowClick = false;
        shareArrowClick = false;
        arrowClick = true;
        settingPop = true;
        background.setPosition(sf::Vector2f(camPos.x, camPos.y));
        I_LOG("cam arrow click");
      }
      else if (shareArrow.onClick(event, mousePosView, wnd)) {
        shareArrowClick = true;
        micArrowClick = false;
        camArrowClick = false;
        arrowClick = true;
        settingPop = true;
        background.setPosition(sf::Vector2f(sharePos.x, sharePos.y));
        I_LOG("share arrow click");
      }
      if (background.getGlobalBounds().contains(mousePosView)) {
        seekbar.eventProcess(event, wnd);
        if (micArrowClick) {
          if (micDrop.eventProcess(event, mousePosView, wnd, false)) {
            I_LOG("mic labal is {}", WstrConv.to_bytes(micDrop.getSelectedLabel()));
          }
          if (spDrop.eventProcess(event, mousePosView, wnd, false)) {
            I_LOG("speaker labal is {}", WstrConv.to_bytes(spDrop.getSelectedLabel()));
          }
        }
        if (camArrowClick) {
          if(camDrop.eventProcess(event, mousePosView, wnd, false)) {
            I_LOG("cam labal is {}", WstrConv.to_bytes(camDrop.getSelectedLabel()));
          }
        }
        if (shareArrowClick) {
          if (screenDrop.eventProcess(event, mousePosView, wnd, false)) {
            I_LOG("screen labal is {}", WstrConv.to_bytes(screenDrop.getSelectedLabel()));
          }
          if (windowDrop.eventProcess(event, mousePosView, wnd, false)) {
            I_LOG("window labal is {}", WstrConv.to_bytes(windowDrop.getSelectedLabel()));
          }
        }
      }
      else if (sense.getGlobalBounds().contains(mousePosView)) {
        //do nothing
      }
      else {
        if (!arrowClick) {
          if (settingPop) settingPop = false;
        }
        if (isFull && settingPop) settingPop = false;
        micArrowClick = false;
        camArrowClick = false;
        shareArrowClick = false;
        setCursor(wnd, sf::Cursor::Arrow);
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
      meetingDescribe.render(wnd);
      if (settingPop) {
        wnd->draw(background);
        if (micArrowClick) {
          wnd->draw(seekbarDescribe);
          wnd->draw(micDescribe);
          wnd->draw(spDescribe);
          seekbar.render(wnd);
          micDrop.render(wnd);
          spDrop.render(wnd);
        }
        else if (camArrowClick) {
          wnd->draw(camDescribe);
          camDrop.render(wnd);
        }
        else if (shareArrowClick) {
          wnd->draw(screenDescribe);
          wnd->draw(windowDescribe);
          screenDrop.render(wnd);
          windowDrop.render(wnd);
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