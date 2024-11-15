#include "seeker/logger.h"
#include "seeker/loggerApi.h"
#include "component/module.h"

#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
  // 创建一个无边框窗口
  sf::RenderWindow window(sf::VideoMode(1280, 720), "无边框窗口", sf::Style::None);
  int wndWidth = window.getSize().x;
  int wndHeight = window.getSize().y;
  // 窗口位置和拖动状态
  sf::Vector2i dragOffset;
  bool isDragging = false;

  // 获取屏幕尺寸
  sf::Vector2i screenSize;
  screenSize.x = sf::VideoMode::getDesktopMode().width;
  screenSize.y = sf::VideoMode::getDesktopMode().height;
  I_LOG("window size is {}:{}", wndWidth, wndHeight);

  // 计算窗口移动允许的边界
  int margin = 40; // 边缘留出的空间
  int maxX = screenSize.x - margin; // 窗口左上角最大x坐标
  int minX = 0 - (wndWidth - margin); // 窗口左上角最小x坐标
  int maxY = screenSize.y - margin * 3; // 窗口左上角最大y坐标
  int minY = 0 - (wndHeight - margin); // 窗口左上角最小y坐标
  I_LOG("min:max x={}:{}, y={}:{}", minX, maxX, minY, maxY);
  using namespace alllink;
  // 创建一个方块
  VariableStateRectangleModule square;
  square.init(50, 30, wndWidth - 100, 0);
  square.setShape(10, 10, sf::Color(0, 0, 0, 0), sf::Color::Black, 1);
  square.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));

  // 创建一条横线
  VariableStateVertxModule horizontalLine;
  horizontalLine.init(50, 30, wndWidth - 150, 0);
  horizontalLine.setVer({
    sf::Vertex(sf::Vector2f(wndWidth - 150 + 19, 15), sf::Color::Black),
      sf::Vertex(sf::Vector2f(wndWidth - 150 + 31, 15), sf::Color::Black)
    });
  horizontalLine.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));

  // 创建一个交叉（×）
  VariableStateVertxModule cross;
  cross.init(50, 30, wndWidth - 50, 0);
  
  cross.setVer({
    sf::Vertex(sf::Vector2f(wndWidth - 50 + 19, 9), sf::Color::Black),
    sf::Vertex(sf::Vector2f(wndWidth - 50 + 31, 21), sf::Color::Black),
    sf::Vertex(sf::Vector2f(wndWidth - 50 + 31, 9), sf::Color::Black),
    sf::Vertex(sf::Vector2f(wndWidth - 50 + 19, 21), sf::Color::Black)
      
    });
  cross.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));

  // 主循环
  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      sf::Vector2i mousePosWin = sf::Mouse::getPosition(window);
      sf::Vector2f mousePosView = window.mapPixelToCoords(mousePosWin);
      if (square.onClick(event, mousePosView, &window)) {

      }
      if (horizontalLine.onClick(event, mousePosView, &window)) {

      }
      if (cross.onClick(event, mousePosView, &window)) {
        window.close();
      }

      // 处理鼠标按下事件
      if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
          // 获取鼠标位置
          sf::Vector2i mousePosition = sf::Mouse::getPosition(window);

          //判断鼠标是否处于可移动位置
          if (mousePosition.x >= 0 && mousePosition.x <= screenSize.x &&
            mousePosition.y >= 0 && mousePosition.y <= 30) {

            isDragging = true; // 开始拖动
            // 计算拖动偏移，使用窗口的当前位置
            dragOffset = sf::Mouse::getPosition() - window.getPosition();
          }
        }
      }

      // 处理鼠标释放事件
      if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
          isDragging = false; // 停止拖动
        }
      }

      // 处理鼠标移动事件
      if (event.type == sf::Event::MouseMoved) {
        if (isDragging) {
          // 更新窗口位置，确保使用的是全局鼠标位置
          sf::Vector2i newPosition = sf::Mouse::getPosition() - dragOffset;
          // 确保窗口不会超出屏幕边缘
          if (newPosition.x < minX) newPosition.x = minX;
          else if (newPosition.x > maxX) newPosition.x = maxX;
          if (newPosition.y < minY) newPosition.y = minY;
          else if (newPosition.y > maxY) newPosition.y = maxY;
          I_LOG("windows new position {}:{}", newPosition.x, newPosition.y);

          window.setPosition(newPosition);
        }
      }
    }

    // 清空窗口并绘制内容
    window.clear(sf::Color::White);

    // 绘制方块
    square.render(&window);

    // 绘制横线
    horizontalLine.render(&window);

    // 绘制交叉（×）
    cross.render(&window);

    window.display();
  }

  return 0;
}
