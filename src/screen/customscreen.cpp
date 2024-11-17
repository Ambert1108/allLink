#include "screen/screensink.h"

namespace alllink {
	CustomScreen::CustomScreen(sf::VideoMode mode, const sf::String& title, sf::Image icon, int style)
		: BaseScreen(mode, title, icon, sf::Style::None, sf::ContextSettings()), 
      style_(Style(style)), isDragging(false), isDesktop(false) {
		wndSize_ = mode;
		hwnd = this->getSystemHandle();
		SetWindowLongPtr(hwnd, GWL_STYLE, WS_MINIMIZEBOX);
		SetWindowText(hwnd, title.toWideString().c_str());
    HRGN hRgn = CreateRoundRectRgn(0, 0, wndSize_.width, wndSize_.height, 15, 15);
    SetWindowRgn(hwnd, hRgn, TRUE);
		ShowWindow(hwnd, 1);
		screenSize.x = sf::VideoMode::getDesktopMode().width;
		screenSize.y = sf::VideoMode::getDesktopMode().height;
		// 计算窗口移动允许的边界
		int margin = 40; // 边缘留出的空间
		maxX = screenSize.x - margin; // 窗口左上角最大x坐标
		minX = 0 - (wndSize_.width - margin); // 窗口左上角最小x坐标
		maxY = screenSize.y - margin * 3; // 窗口左上角最大y坐标
		minY = 0 - (wndSize_.height - margin); // 窗口左上角最小y坐标
		switch (style_) {
		case All:
			square.set(50, 30, wndSize_.width - 100, 0);
			square.setShapeSize(10, 10);
			square.setShapeColor(sf::Color(0, 0, 0, 0), sf::Color::Black, 1);
			square.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
			[[fallthrough]];
		case Minisize:
			horizontalLine.set(50, 30, wndSize_.width - 150, 0);
			horizontalLine.setVer({
				sf::Vertex(sf::Vector2f(wndSize_.width - 150 + 19, 15), sf::Color::Black),
				sf::Vertex(sf::Vector2f(wndSize_.width - 150 + 31, 15), sf::Color::Black)
				});
			horizontalLine.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
			[[fallthrough]];
		case Close:
			cross.set(50, 30, wndSize_.width - 50, 0);
			cross.setVer({
				sf::Vertex(sf::Vector2f(wndSize_.width - 50 + 19, 9), sf::Color::Black),
				sf::Vertex(sf::Vector2f(wndSize_.width - 50 + 31, 21), sf::Color::Black),
				sf::Vertex(sf::Vector2f(wndSize_.width - 50 + 31, 9), sf::Color::Black),
				sf::Vertex(sf::Vector2f(wndSize_.width - 50 + 19, 21), sf::Color::Black)
				});
			cross.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
			break;
		}
	}

	void CustomScreen::checkStatus(sf::Event& event) {
    auto mousePosView = getMousePosition();
    // 按下窗口最小化，窗口失焦隐藏
    if (style_ >= Minisize && horizontalLine.onClick(event, mousePosView, this)) {
      ShowWindow(hwnd, SW_MINIMIZE);
    }

    // 按下窗口最大化
    if (style_ == All && square.onClick(event, mousePosView, this)) {
      isDesktop = !isDesktop;
      if (isDesktop) {
        sf::FloatRect visibleArea(0.f, 0.f, sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);
        this->setView(sf::View(visibleArea));
        this->setSize(sf::Vector2u(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height));
        this->setPosition(sf::Vector2i(0, 0));

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
      }
      else {
        sf::FloatRect visibleArea(0.f, 0.f, wndSize_.width, wndSize_.height);
        this->setView(sf::View(visibleArea));
        this->setSize(sf::Vector2u(wndSize_.width, wndSize_.height));
        this->setPosition(wndPos);

        square.set(50, 30, wndSize_.width - 100, 0);
        square.setShapePos(square.getPosition().x + 20, square.getPosition().y + 10);

        horizontalLine.set(50, 30, wndSize_.width - 150, 0);
        horizontalLine.setVer({
          sf::Vertex(sf::Vector2f(wndSize_.width - 150 + 19, 15), sf::Color::Black),
          sf::Vertex(sf::Vector2f(wndSize_.width - 150 + 31, 15), sf::Color::Black)
          });

        cross.set(50, 30, wndSize_.width - 50, 0);
        cross.setVer({
          sf::Vertex(sf::Vector2f(wndSize_.width - 50 + 19, 9), sf::Color::Black),
          sf::Vertex(sf::Vector2f(wndSize_.width - 50 + 31, 21), sf::Color::Black),
          sf::Vertex(sf::Vector2f(wndSize_.width - 50 + 31, 9), sf::Color::Black),
          sf::Vertex(sf::Vector2f(wndSize_.width - 50 + 19, 21), sf::Color::Black)
          });
      }
    }

    // 窗口重新获得焦点，显示窗口
    if (event.type == sf::Event::GainedFocus) {
      ShowWindow(hwnd, SW_SHOW);
    }

    // 按下窗口关闭
    if (cross.onClick(event, mousePosView, this)) {
      //this->close();
      needClose();
    }
    else {
      // 处理鼠标按下事件
      if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
          // 获取鼠标位置
          sf::Vector2i mousePosition = sf::Mouse::getPosition(*this);

          //判断鼠标是否处于可移动位置
          if (mousePosition.x >= 0 && mousePosition.x <= wndSize_.width - 150 &&
            mousePosition.y >= 0 && mousePosition.y <= 30) {

            isDragging = true; // 开始拖动
            // 计算拖动偏移，使用窗口的当前位置
            dragOffset = sf::Mouse::getPosition() - this->getPosition();
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
          this->setPosition(newPosition);
        }
      }
    }
	}
}