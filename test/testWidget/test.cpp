#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "component/module.h"

int main() {
  using namespace alllink;
  sf::RenderWindow* wnd = new sf::RenderWindow(
    sf::VideoMode(640,480),
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
  src1.setImageColor(sf::Color(124, 171, 214));

  HorizonGraphicTextsModule src2;
  src2.init(140, 42, 95, 211);
  src2.setSource(20, font2File, image2File);
  src2.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
  src2.setText(L"请登录", sf::Color(0, 0, 0));
  src2.setImage();
  src2.setFill(false);
  
  while (wnd->isOpen()) {
    while (wnd->pollEvent(event)) {
      sf::Vector2i mousePosWin = sf::Mouse::getPosition(*wnd);
      sf::Vector2f mousePosView = wnd->mapPixelToCoords(mousePosWin);
      if (src1.onClick(event, mousePosView, wnd)) {
        std::cout << "创建会议" << std::endl;
      }
      else if (src2.onClick(event, mousePosView, wnd)) {
        std::cout << "请登录" << std::endl;
      }
      switch (event.type) {
      case sf::Event::Closed:
        wnd->close();
        break;
  
      case sf::Event::KeyPressed:
        if (event.key.code == sf::Keyboard::Escape) {
          wnd->close();
        }
        break;
      }
    }
  
    wnd->clear(sf::Color(240, 240, 240));
    src1.render(wnd);
    src2.render(wnd);
    wnd->display();
  }
	return 0;
}