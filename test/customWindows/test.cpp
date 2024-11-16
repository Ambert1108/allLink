#include "seeker/logger.h"
#include "seeker/loggerApi.h"
#include "component/module.h"

#include <SFML/Graphics.hpp>
#include <Windows.h>
#include <iostream>

using namespace alllink;
sf::RenderWindow window;
HWND hwnd;
// 窗口位置和拖动状态
sf::Vector2i dragOffset;
bool isDragging = false;
sf::Vector2i screenSize;
int maxX = 0;
int minX = 0;
int maxY = 0;
int minY = 0;
VariableStateRectangleModule square;
VariableStateVertxModule horizontalLine;
VariableStateVertxModule cross;
sf::Vector2f wndSize;

void initWindow(int width, int height, sf::String title) {
  // 创建一个无边框窗口
  window.create(sf::VideoMode(width, height), title, sf::Style::None);
  wndSize.x = window.getSize().x;
  wndSize.y = window.getSize().y;

  // 获取窗口句柄
  hwnd = window.getSystemHandle();
  SetWindowLongPtr(hwnd, GWL_STYLE, WS_MINIMIZEBOX);
  SetWindowText(hwnd, title.toWideString().c_str());
  int radius = 20; // 圆角半径
  HRGN hRgn = CreateRoundRectRgn(0, 0, width, height, radius, radius);
  SetWindowRgn(hwnd, hRgn, TRUE);
  ShowWindow(hwnd, 1);

  // 获取屏幕尺寸
  screenSize.x = sf::VideoMode::getDesktopMode().width;
  screenSize.y = sf::VideoMode::getDesktopMode().height;
  I_LOG("window size is {}:{}", wndSize.x, wndSize.y);

  // 计算窗口移动允许的边界
  int margin = 40; // 边缘留出的空间
  maxX = screenSize.x - margin; // 窗口左上角最大x坐标
  minX = 0 - (wndSize.x - margin); // 窗口左上角最小x坐标
  maxY = screenSize.y - margin * 3; // 窗口左上角最大y坐标
  minY = 0 - (wndSize.y - margin); // 窗口左上角最小y坐标
  I_LOG("min:max x={}:{}, y={}:{}", minX, maxX, minY, maxY);
  // 创建一个方块
  square.set(50, 30, wndSize.x - 100, 0);
  square.setShapeSize(10, 10);
  square.setShapeColor(sf::Color(0, 0, 0, 0), sf::Color::Black, 1);
  square.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));

  // 创建一条横线
  horizontalLine.set(50, 30, wndSize.x - 150, 0);
  horizontalLine.setVer({
    sf::Vertex(sf::Vector2f(wndSize.x - 150 + 19, 15), sf::Color::Black),
      sf::Vertex(sf::Vector2f(wndSize.x - 150 + 31, 15), sf::Color::Black)
    });
  horizontalLine.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));

  // 创建一个交叉（×）
  cross.set(50, 30, wndSize.x - 50, 0);
  cross.setVer({
    sf::Vertex(sf::Vector2f(wndSize.x - 50 + 19, 9), sf::Color::Black),
    sf::Vertex(sf::Vector2f(wndSize.x - 50 + 31, 21), sf::Color::Black),
    sf::Vertex(sf::Vector2f(wndSize.x - 50 + 31, 9), sf::Color::Black),
    sf::Vertex(sf::Vector2f(wndSize.x - 50 + 19, 21), sf::Color::Black)
    });
  cross.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
}

int main() {
  initWindow(1280, 720, L"无边框窗口");
  sf::View view = window.getDefaultView(); // 获取默认视图
  view.reset(sf::FloatRect(0, 0, 1280, 720)); // 重置视图到原始大小
  window.setView(view);
  sf::RectangleShape shape;
  shape.setSize(sf::Vector2f(100, 100));
  shape.setFillColor(sf::Color::Green);
  shape.setPosition(sf::Vector2f(590, 310));

  //window.setView(sf::View(sf::FloatRect(0, 0, 1280, 30)));

  auto wndPos = window.getPosition();
  bool isDesktop = false;
  float wr = 0.f;
  float hr = 0.f;
  // 主循环
  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      sf::Vector2i mousePosWin = sf::Mouse::getPosition(window);
      sf::Vector2f mousePosView = window.mapPixelToCoords(mousePosWin);

      // 按下窗口最小化，窗口失焦隐藏
      if (horizontalLine.onClick(event, mousePosView, &window)) {
        ShowWindow(hwnd, SW_MINIMIZE);
      }

      // 按下窗口最大化
      if (square.onClick(event, mousePosView, &window)) {
        isDesktop = !isDesktop;
        if (isDesktop) {
          //initWindow(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);
          wr = (float)sf::VideoMode::getDesktopMode().width / wndSize.x;
          hr = (float)sf::VideoMode::getDesktopMode().height / wndSize.y;
          sf::FloatRect visibleArea(0.f, 0.f, sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);
          window.setView(sf::View(visibleArea));
          window.setSize(sf::Vector2u(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height));
          window.setPosition(sf::Vector2i(0, 0));
          
          square.set(50, 30, sf::VideoMode::getDesktopMode().width - 100, 0);
          square.setShapePos(square.getPosition().x + 20, square.getPosition().y + 10);
          
          horizontalLine.set(50, 30, sf::VideoMode::getDesktopMode().width - 150, 0);
          horizontalLine.setVer({
            sf::Vertex(sf::Vector2f(sf::VideoMode::getDesktopMode().width - 150 + 19, 15), sf::Color::Black),
            sf::Vertex(sf::Vector2f(sf::VideoMode::getDesktopMode().width - 150 + 31, 15), sf::Color::Black)
            });
          cross.set(50, 30, sf::VideoMode::getDesktopMode().width - 50, 0);
          cross.setVer({
            sf::Vertex(sf::Vector2f(sf::VideoMode::getDesktopMode().width - 50 + 19, 9), sf::Color::Black),
            sf::Vertex(sf::Vector2f(sf::VideoMode::getDesktopMode().width - 50 + 31, 21), sf::Color::Black),
            sf::Vertex(sf::Vector2f(sf::VideoMode::getDesktopMode().width - 50 + 31, 9), sf::Color::Black),
            sf::Vertex(sf::Vector2f(sf::VideoMode::getDesktopMode().width - 50 + 19, 21), sf::Color::Black)
            });

          shape.setScale(wr, hr);
          shape.setPosition(sf::Vector2f(shape.getPosition().x * wr, shape.getPosition().y * hr));
        }
        else {
          //initWindow(1280, 720);
          sf::FloatRect visibleArea(0.f, 0.f, 1280, 720);
          window.setView(sf::View(visibleArea));
          window.setSize(sf::Vector2u(1280, 720));
          window.setPosition(wndPos);

          square.set(50, 30, 1280 - 100, 0);
          square.setShapePos(square.getPosition().x + 20, square.getPosition().y + 10);
          
          horizontalLine.set(50, 30, 1280 - 150, 0);
          horizontalLine.setVer({
            sf::Vertex(sf::Vector2f(1280 - 150 + 19, 15), sf::Color::Black),
            sf::Vertex(sf::Vector2f(1280 - 150 + 31, 15), sf::Color::Black)
            });
          
          cross.set(50, 30, 1280 - 50, 0);
          cross.setVer({
            sf::Vertex(sf::Vector2f(1280 - 50 + 19, 9), sf::Color::Black),
            sf::Vertex(sf::Vector2f(1280 - 50 + 31, 21), sf::Color::Black),
            sf::Vertex(sf::Vector2f(1280 - 50 + 31, 9), sf::Color::Black),
            sf::Vertex(sf::Vector2f(1280 - 50 + 19, 21), sf::Color::Black)
            });

          shape.setScale(1, 1);
          shape.setPosition(sf::Vector2f(590, 310));
        }
      }

      // 按下窗口关闭
      if (cross.onClick(event, mousePosView, &window)) {
        window.close();
      }

      // 窗口重新获得焦点，显示窗口
      if (event.type == sf::Event::GainedFocus) {
        ShowWindow(hwnd, SW_SHOW);
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

    window.draw(shape);

    window.display();
  }

  return 0;
}