#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "component/module.h"
#include "component/widget.h"

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
  src2.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225));
  src2.setText(L"请登录", sf::Color(0, 0, 0));
  src2.setImage();
  src2.setFill(false);

  //InputBoxMoudule input;
  //input.init(228, 32, 20, 20);
  //input.setText(font2File);
  //input.setColor(sf::Color(215, 215, 215), sf::Color(205, 205, 205), sf::Color(255, 255, 255));

  EnterDescriptionWidget inputWidget(264, 94, 20, 20);
  inputWidget.setInput(font2File);
  inputWidget.setDescription(font2File, L"服务器地址");
  
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
      inputWidget.eventProcess(event, wnd);
      //else if (input.onClick(event, mousePosView, wnd)) {
      //  input.setActive(true);
      //}
      //else {
      //  if (event.type == sf::Event::MouseButtonPressed
      //    && event.key.code == sf::Mouse::Left) {
      //    input.setActive(false);
      //  }
      //}
      //if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S) {
      //  I_LOG("save input:{}", input.getEnterText());
      //}
      //input.eventProcess(event);
    }
    
    wnd->clear(sf::Color(240, 240, 240));
    src1.render(wnd);
    src2.render(wnd);
    inputWidget.render(wnd);
    //I_LOG("draw, color is {}", input.getFillColor().toInteger());
    //input.render(wnd);
    wnd->display();
  }
	return 0;
}