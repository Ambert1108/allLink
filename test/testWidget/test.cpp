#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "component/module.h"
#include "component/widget.h"

int main() {
  using namespace alllink;
  sf::RenderWindow* wnd = new sf::RenderWindow(
    sf::VideoMode(1280, 720),
    "widget",
    sf::Style::Default);
  wnd->setFramerateLimit(60);
  sf::Event event{};
  std::string fontFile = "./resources/fonts/fzch.ttf";
  std::string font2File = "./resources/fonts/msyh.ttc";
  std::string image1File = "./resources/comp/start/create_meeting.png";
  std::string image2File = "./resources/comp/start/start_login.png";
  VerticalGraphicTextsModule src1;
  src1.init(138, 130, 425, 65);
  src1.setSource(15, fontFile, image1File);
  src1.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  src1.setText(L"创建会议", sf::Color(0, 0, 0));
  //src1.setImageSize(16, 16);
  src1.setImage();
  src1.setImageColor(sf::Color(117, 188, 255));

  HorizonGraphicTextsModule src2(true);
  src2.init(140, 42, 95, 211);
  src2.setSource(20, font2File, image2File);
  src2.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225));
  src2.setText(L"请登录", sf::Color(0, 0, 0));
  src2.setImage();
  src2.setImageColor(sf::Color(117, 188, 255));
  src2.setFill(false);

  //InputBoxMoudule input;
  //input.init(228, 32, 20, 20);
  //input.setText(font2File);
  //input.setColor(sf::Color(215, 215, 215), sf::Color(205, 205, 205), sf::Color(255, 255, 255));

  EnterDescriptionWidget inputWidget(264, 94, 20, 20);
  inputWidget.setInput(font2File);
  inputWidget.setDescription(font2File, L"服务器地址");

  DropDescriptionWidget dropWidget(264, 240, 640, 20, font2File);
  dropWidget.setDescription(L"J组公网信令", sf::Color::Black);
  dropWidget.setDropList(4);
  dropWidget.addLabel(L"J组公网信令", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));
  dropWidget.addLabel(L"J组内网信令", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));
  dropWidget.addLabel(L"X组公网信令", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));
  dropWidget.addLabel(L"X组内网信令", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));

  TextRectangle textRect;
  textRect.init(98, 48, 200, 400);
  textRect.setText(font2File, L"登录", sf::Color::White);
  textRect.setStateColor(sf::Color(104, 141, 196), sf::Color(213, 229, 240));
  std::string saveText{};

  ClickTextRoundRectangle testRect(sf::Color(117, 188, 255), 2);
  testRect.init(260, 40, 640, 600, 8.f);
  testRect.setText(font2File, L"测试", sf::Color::Black);
  testRect.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));


  while (wnd->isOpen()) {
    while (wnd->pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        wnd->close();
        break;
      }
      sf::Vector2i mousePosWin = sf::Mouse::getPosition(*wnd);
      sf::Vector2f mousePosView = wnd->mapPixelToCoords(mousePosWin);
      if (src1.onClick(event, mousePosView, wnd)) {
        std::cout << "创建会议" << std::endl;
      }
      else if (src2.onClick(event, mousePosView, wnd)) {
        std::cout << "请登录" << std::endl;
      }
      else if (textRect.onClick(event, mousePosView, wnd) && textRect.getActive() && !saveText.empty()) {
        std::cout << "进行登录" << std::endl;
      }
      inputWidget.eventProcess(event, wnd);

      if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
        std::cout << "aaa" << std::endl;
        if (!inputWidget.getInputActive()) {
          std::cout << "bbb" << std::endl;
          saveText = inputWidget.getInput();
          if (saveText.empty()) textRect.setActive(false);
          else {
            I_LOG("save:{}", saveText);
            textRect.setActive(true);
          }
        }
      }
      
      if (dropWidget.eventProcess(event, wnd)) {
        I_LOG("drop choose: {}", WstrConv.to_bytes(dropWidget.getSelectedLabel()));
      }

      testRect.onClick(event, mousePosView, wnd);
    }
    
    wnd->clear(sf::Color(240, 240, 240));
    testRect.render(wnd);
    src1.render(wnd);
    src2.render(wnd);
    inputWidget.render(wnd);
    textRect.render(wnd);
    dropWidget.render(wnd);
    wnd->display();
  }
	return 0;
}