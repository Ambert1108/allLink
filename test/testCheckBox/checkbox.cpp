#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "component/module.h"
#include "component/widget.h"

int main() {
  using namespace alllink;
  sf::RenderWindow* wnd = new sf::RenderWindow(
    sf::VideoMode(640, 480),
    "test",
    sf::Style::Default);
  wnd->setFramerateLimit(60);
  sf::Event event{};
  std::string fontFile = "./resources/fonts/fzch.ttf";
  std::string font2File = "./resources/fonts/msyh.ttc";
  std::string imageFile = "./resources/comp/start/yes.png";
  CheckBoxModule checkBox;
  checkBox.init(sf::Vector2f(16, 16), sf::Vector2f(50, 100));
  checkBox.setDescribe(font2File, L"音频MCU", 14);
  checkBox.setBox(sf::Color::White, imageFile);
  checkBox.setInteractiveColor(sf::Color(173, 173, 173), sf::Color(37, 194, 94));

  while (wnd->isOpen()) {
    while (wnd->pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        wnd->close();
        break;
      }
      sf::Vector2i mousePosWin = sf::Mouse::getPosition(*wnd);
      sf::Vector2f mousePosView = wnd->mapPixelToCoords(mousePosWin);
      if (checkBox.onClick(event, mousePosView, wnd)) {
        if(checkBox.data()) I_LOG("check");
      }
    }

    wnd->clear(sf::Color(240, 240, 240));
    checkBox.render(wnd);
    wnd->display();
  }
	return 0;
}