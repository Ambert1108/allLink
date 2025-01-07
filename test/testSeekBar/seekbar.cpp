#include "component/module.h"
#include <cmath>
#include <vector>

int main() {
  sf::RenderWindow window(sf::VideoMode(800, 600), "Rounded Rectangle Example");
  window.setFramerateLimit(60);

  using namespace alllink;
  std::string fontFile = "./resources/fonts/msyh.ttc";
  SeekBarModule seekbar, verSeekbar;
  seekbar.init(sf::Vector2f(130, 5), 10, sf::Vector2f(330, 215), sf::Color::White, sf::Color(68, 118, 235));
  seekbar.setText(fontFile, 0, 14, sf::Color::Black);
  verSeekbar.init(sf::Vector2f(5, 150), 10, sf::Vector2f(200, 40), sf::Color::White, sf::Color(68, 118, 235), false);
  verSeekbar.setText(fontFile, 2, 14, sf::Color::Black);

  VariableStateVertxFillModule arrow;
  arrow.set(8, 30, 400, 500);
  arrow.setVer({ 
    sf::Vertex(sf::Vector2f(400, 520), sf::Color::Black),
    sf::Vertex(sf::Vector2f(400 + 4, 510), sf::Color::Black),
    sf::Vertex(sf::Vector2f(400 + 4, 510), sf::Color::Black),
    sf::Vertex(sf::Vector2f(400 + 8, 520), sf::Color::Black) });
  arrow.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));

  DropListModule drop;
  drop.init(180, 50, 318, 260, 40, fontFile);
  drop.addLabel(L"麦克风阵列");
  drop.addLabel(L"罗技麦克风");
  drop.addLabel(L"麦克风");
  drop.setShow(true);

  VariableStateFillModule background(sf::Color::Transparent, 2);
  background.setSize(sf::Vector2f(200, 300), 5);
  background.setPosition(308, 198);
  background.setFillColor(sf::Color(220, 220, 220));

  bool micSettingPop = false;

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      bool arrowClick = false;
      if (event.type == sf::Event::Closed)
        window.close();
      sf::Vector2i mousePosWin = sf::Mouse::getPosition(window);
      sf::Vector2f mousePosView = window.mapPixelToCoords(mousePosWin);
      verSeekbar.eventProcess(event, &window);
      if (arrow.onClick(event, mousePosView, &window)) {
        arrowClick = true;
        micSettingPop = !micSettingPop;
      }
      if (background.getGlobalBounds().contains(mousePosView)) {
        seekbar.eventProcess(event, &window);
        if (drop.eventProcess(event, mousePosView, &window, false)) {
          I_LOG("labal is {}", WstrConv.to_bytes(drop.getSelectedLabel()));
        }
      }
      else {
        if (event.type == sf::Event::MouseButtonReleased
          && event.mouseButton.button == sf::Mouse::Left
          && !arrowClick) {
          if (micSettingPop) micSettingPop = false;
        }
      }
    }
    if (seekbar.update(&window)) {
      I_LOG("horseekbar data is {}", seekbar.data());
    }
    if (verSeekbar.update(&window)) {
      I_LOG("verSeekbar data is {}", verSeekbar.data());
    }
    window.clear(sf::Color(240, 240, 240));
    if (micSettingPop) {
      window.draw(background);  
      seekbar.render(&window);
      drop.render(&window);
    }
    verSeekbar.render(&window);
    arrow.render(&window);
    window.display(); 
  }

  return 0;
}