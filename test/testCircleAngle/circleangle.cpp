#include "component/module.h"
#include <cmath>
#include <vector>

class RoundedRectangle : public sf::Drawable {
public:
	RoundedRectangle() 
		: m_size(100, 50), m_radius(15) {

		// 创建角落的圆形
		for (int i = 0; i < 4; ++i) {
			sf::CircleShape corner(m_radius);
			corner.setPointCount(30); // 增加圆的平滑度
			corner.setFillColor(sf::Color::Transparent); // 设置填充颜色为透明
			m_corners.push_back(corner);
		}

		// 创建矩形中间部分
		m_center.setSize(sf::Vector2f(m_size.x - m_radius * 2, m_size.y));
		m_center.setFillColor(sf::Color::White); // 填充颜色
		m_center.setOutlineColor(sf::Color::Transparent); // 边缘颜色
		m_center.setOutlineThickness(0); // 边缘厚度

		m_center2.setSize(sf::Vector2f(m_size.x, m_size.y - m_radius * 2));
		m_center2.setFillColor(sf::Color::White); // 填充颜色
		m_center2.setOutlineColor(sf::Color::Transparent); // 边缘颜色
		m_center2.setOutlineThickness(0); // 边缘厚度

		updateCorners();
	}

	RoundedRectangle(sf::Vector2f size, float radius)
		: m_size(size), m_radius(radius) {

		// 创建角落的圆形
		for (int i = 0; i < 4; ++i) {
			sf::CircleShape corner(m_radius);
			corner.setPointCount(30); // 增加圆的平滑度
			corner.setFillColor(sf::Color::Transparent); // 设置填充颜色为透明
			m_corners.push_back(corner);
		}

		// 创建矩形中间部分
		m_center.setSize(sf::Vector2f(m_size.x - m_radius * 2, m_size.y));
		m_center.setFillColor(sf::Color::White); // 填充颜色
		m_center.setOutlineColor(sf::Color::Transparent); // 边缘颜色
		m_center.setOutlineThickness(0); // 边缘厚度

		m_center2.setSize(sf::Vector2f(m_size.x, m_size.y - m_radius * 2));
		m_center2.setFillColor(sf::Color::White); // 填充颜色
		m_center2.setOutlineColor(sf::Color::Transparent); // 边缘颜色
		m_center2.setOutlineThickness(0); // 边缘厚度

		updateCorners();
	}

	void setPosition(float x, float y) {
		m_position = { x, y };
		updateCorners(); // 更新角落的位置信息
		m_center.setPosition(x + m_radius, y);
		m_center2.setPosition(x, y + m_radius);
	}

	void setSize(sf::Vector2f size, float radius) {
		m_size = size;
		m_radius = radius;
		m_center.setSize(sf::Vector2f(m_size.x - m_radius * 2, m_size.y));
		m_center2.setSize(sf::Vector2f(m_size.x, m_size.y - m_radius * 2));
		updateCorners();
	}

	void setFillColor(const sf::Color& color) {
		m_fillColor = color;
		m_center.setFillColor(color);
		m_center2.setFillColor(color);
		for (auto& corner : m_corners) {
			corner.setFillColor(color);
		}
	}

	void setOutlineColor(const sf::Color& color) {
		m_outlineColor = color;
		m_center.setOutlineColor(color);
		m_center2.setOutlineColor(color);
		for (auto& corner : m_corners) {
			corner.setOutlineColor(color);
		}
	}

	void setOutlineThickness(float thickness) {
		m_outlineThickness = thickness;
		m_center.setOutlineThickness(thickness);
		m_center2.setOutlineThickness(thickness);
		for (auto& corner : m_corners) {
			corner.setOutlineThickness(thickness);
		}
	}

	sf::Vector2f getPosition() const {
		return m_position;
	}

	sf::FloatRect getGlobalBounds() const {
		return sf::FloatRect(m_position.x, m_position.y, m_size.x, m_size.y);
	}

	sf::Vector2f getSize() const {
		return m_size;
	}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		// 绘制四个圆角
		for (const auto& corner : m_corners) {
			target.draw(corner, states);
		}

		// 绘制中间部分
		target.draw(m_center, states);
		target.draw(m_center2, states);

	}

private:
	void updateCorners() {
		// 设置角落的位置
		m_corners[0].setPosition(m_position.x, m_position.y); // 左上角
		m_corners[1].setPosition(m_position.x + m_size.x - m_radius * 2, m_position.y); // 右上角
		m_corners[2].setPosition(m_position.x, m_position.y + m_size.y - m_radius * 2); // 左下角
		m_corners[3].setPosition(m_position.x + m_size.x - m_radius * 2, m_position.y + m_size.y - m_radius * 2); // 右下角
	}

	sf::Vector2f m_size;
	float m_radius;
	sf::Vector2f m_position; // 存储位置
	sf::Color m_fillColor; // 填充颜色
	sf::Color m_outlineColor; // 边缘颜色
	float m_outlineThickness; // 边缘厚度
	sf::RectangleShape m_center;
	sf::RectangleShape m_center2;
	std::vector<sf::CircleShape> m_corners;
};

class CircleRectangle : public sf::Drawable {
public:
	CircleRectangle(sf::Vector2f size, float radius)
		: round(size, radius), fill(size, radius) {};

	void setPosition(float x, float y) {
		round.setPosition(x, y);
		fill.setPosition(x, y);
	}

	void setSize(sf::Vector2f size, float radius) {
		round.setSize(size, radius);
		fill.setSize(size, radius);
	}

	void setFillColor(const sf::Color& color) {
		round.setFillColor(color);
		fill.setFillColor(color);
	}

	void setOutlineColor(const sf::Color& color) {
		round.setOutlineColor(color);
	}

	void setOutlineThickness(float thickness) {
		round.setOutlineThickness(thickness);
	}

	sf::Vector2f getPosition() const {
		return round.getPosition();
	}

	sf::FloatRect getGlobalBounds() const {
		return round.getGlobalBounds();
	}

	sf::Vector2f getSize() const {
		return round.getSize();
	}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		target.draw(round);
		target.draw(fill);
	}

private:
	RoundedRectangle round;
	RoundedRectangle fill;
};

int main() {
  sf::RenderWindow window(sf::VideoMode(800, 600), "Rounded Rectangle Example");
  window.setFramerateLimit(10);
  // 创建圆角矩形，设置填充颜色和边缘颜色
  //alllink::CircleRectangle roundedRect(sf::Vector2f(100, 50), 20);
  //alllink::CircleRectangle roundedRect;
	//roundedRect.setSize(sf::Vector2f(300, 150), 18);
  //roundedRect.setPosition(250, 200); // 设置位置
  //roundedRect.setFillColor(sf::Color::White);
  //roundedRect.setOutlineColor(sf::Color::Red);
  //roundedRect.setOutlineThickness(5);

	std::string font2File = "./resources/fonts/msyh.ttc";
	alllink::TextFillRectangle roundedRect(sf::Color::Red, 5);
	roundedRect.init(190, 50, 250, 200, 20.0);
	roundedRect.setColor(sf::Color::White, sf::Color::Red, sf::Color(161, 19, 19));
	roundedRect.setText(font2File, L"离开会议", sf::Color::Black, sf::Color::White);
  int i = 1;
  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
        window.close();
				break;
			}
			sf::Vector2i mousePosWin = sf::Mouse::getPosition(window);
			sf::Vector2f mousePosView = window.mapPixelToCoords(mousePosWin);
			if (roundedRect.onClick(event, mousePosView, &window)) {
				I_LOG("离开会议");
			}
    }
    //if(i < 490) roundedRect.setPosition(++i, 100);
		if (i < 490) {
			i++;
			//roundedRect.setSize(sf::Vector2f(300 + i, 200), 20);
		}
    window.clear();
    //window.draw(roundedRect);
		roundedRect.render(&window);
    window.display();
  }

  return 0;
}
