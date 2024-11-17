#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>

int main() {
  sf::VideoMode vm(400, 200);
  sf::RenderWindow window(vm, "SFML Text Input");

  std::wstring text = L"请输入";
  sf::Text inputText;
  sf::Font font;
  font.loadFromFile("./resources/fonts/msyh.ttc");
  inputText.setFont(font); // 确保这个字体文件在你的项目目录中
  inputText.setString(text);
  inputText.setCharacterSize(24);
  inputText.setFillColor(sf::Color::Black);
  inputText.setPosition(50, 50);

  // 光标位置
  std::size_t cursorPosition = text.length();
  bool showCursor = true;
  int blinkCounter = 0;

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();

      if (event.type == sf::Event::TextEntered && (event.text.unicode > 32 && event.text.unicode <= 126)) {
        if (cursorPosition < text.length()) {
          text.insert(cursorPosition, 1, event.text.unicode);
        }
        else {
          text += event.text.unicode;
        }
        cursorPosition++;
        inputText.setString(text);
      }

      if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Left) {
          if (cursorPosition > 0) {
            cursorPosition--;
          }
        }
        else if (event.key.code == sf::Keyboard::Right) {
          if (cursorPosition < text.length()) {
            cursorPosition++;
          }
        }
        else if (event.key.code == sf::Keyboard::Backspace) {
          std::cout << "1" << std::endl;
          if (cursorPosition > 0) {
            text.erase(cursorPosition - 1, 1);
            cursorPosition--;
            inputText.setString(text);
          }
        }
      }
    }

    blinkCounter++;
    if (blinkCounter >= 3000) {
      showCursor = !showCursor;
      blinkCounter = 0;
    }

    window.clear(sf::Color::White);

    // 绘制文本
    inputText.setPosition(50, 50);
    window.draw(inputText);

    // 绘制光标
    if (showCursor) {
      sf::Vertex cursor[2];
      sf::Vector2f cursorPos = inputText.findCharacterPos(cursorPosition);

      cursor[0].position = sf::Vector2f(cursorPos.x, cursorPos.y);
      cursor[1].position = sf::Vector2f(cursorPos.x, cursorPos.y + inputText.getCharacterSize());

      cursor[0].color = sf::Color::Black;
      cursor[1].color = sf::Color::Black;

      window.draw(cursor, 2, sf::Lines);
    }

    window.display();
  }

  return 0;
}