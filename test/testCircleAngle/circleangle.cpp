//#include "component/module.h"
//#include <cmath>
//#include <vector>
//
//class RoundedRectangle : public sf::Drawable {
//public:
//	RoundedRectangle() 
//		: m_size(100, 50), m_radius(15) {
//
//		// 创建角落的圆形
//		for (int i = 0; i < 4; ++i) {
//			sf::CircleShape corner(m_radius);
//			corner.setPointCount(30); // 增加圆的平滑度
//			corner.setFillColor(sf::Color::Transparent); // 设置填充颜色为透明
//			m_corners.push_back(corner);
//		}
//
//		// 创建矩形中间部分
//		m_center.setSize(sf::Vector2f(m_size.x - m_radius * 2, m_size.y));
//		m_center.setFillColor(sf::Color::White); // 填充颜色
//		m_center.setOutlineColor(sf::Color::Transparent); // 边缘颜色
//		m_center.setOutlineThickness(0); // 边缘厚度
//
//		m_center2.setSize(sf::Vector2f(m_size.x, m_size.y - m_radius * 2));
//		m_center2.setFillColor(sf::Color::White); // 填充颜色
//		m_center2.setOutlineColor(sf::Color::Transparent); // 边缘颜色
//		m_center2.setOutlineThickness(0); // 边缘厚度
//
//		updateCorners();
//	}
//
//	RoundedRectangle(sf::Vector2f size, float radius)
//		: m_size(size), m_radius(radius) {
//
//		// 创建角落的圆形
//		for (int i = 0; i < 4; ++i) {
//			sf::CircleShape corner(m_radius);
//			corner.setPointCount(30); // 增加圆的平滑度
//			corner.setFillColor(sf::Color::Transparent); // 设置填充颜色为透明
//			m_corners.push_back(corner);
//		}
//
//		// 创建矩形中间部分
//		m_center.setSize(sf::Vector2f(m_size.x - m_radius * 2, m_size.y));
//		m_center.setFillColor(sf::Color::White); // 填充颜色
//		m_center.setOutlineColor(sf::Color::Transparent); // 边缘颜色
//		m_center.setOutlineThickness(0); // 边缘厚度
//
//		m_center2.setSize(sf::Vector2f(m_size.x, m_size.y - m_radius * 2));
//		m_center2.setFillColor(sf::Color::White); // 填充颜色
//		m_center2.setOutlineColor(sf::Color::Transparent); // 边缘颜色
//		m_center2.setOutlineThickness(0); // 边缘厚度
//
//		updateCorners();
//	}
//
//	void setPosition(float x, float y) {
//		m_position = { x, y };
//		updateCorners(); // 更新角落的位置信息
//		m_center.setPosition(x + m_radius, y);
//		m_center2.setPosition(x, y + m_radius);
//	}
//
//	void setSize(sf::Vector2f size, float radius) {
//		m_size = size;
//		m_radius = radius;
//		m_center.setSize(sf::Vector2f(m_size.x - m_radius * 2, m_size.y));
//		m_center2.setSize(sf::Vector2f(m_size.x, m_size.y - m_radius * 2));
//		updateCorners();
//	}
//
//	void setFillColor(const sf::Color& color) {
//		m_fillColor = color;
//		m_center.setFillColor(color);
//		m_center2.setFillColor(color);
//		for (auto& corner : m_corners) {
//			corner.setFillColor(color);
//		}
//	}
//
//	void setOutlineColor(const sf::Color& color) {
//		m_outlineColor = color;
//		m_center.setOutlineColor(color);
//		m_center2.setOutlineColor(color);
//		for (auto& corner : m_corners) {
//			corner.setOutlineColor(color);
//		}
//	}
//
//	void setOutlineThickness(float thickness) {
//		m_outlineThickness = thickness;
//		m_center.setOutlineThickness(thickness);
//		m_center2.setOutlineThickness(thickness);
//		for (auto& corner : m_corners) {
//			corner.setOutlineThickness(thickness);
//		}
//	}
//
//	sf::Vector2f getPosition() const {
//		return m_position;
//	}
//
//	sf::FloatRect getGlobalBounds() const {
//		return sf::FloatRect(m_position.x, m_position.y, m_size.x, m_size.y);
//	}
//
//	sf::Vector2f getSize() const {
//		return m_size;
//	}
//
//	void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
//		// 绘制四个圆角
//		for (const auto& corner : m_corners) {
//			target.draw(corner, states);
//		}
//
//		// 绘制中间部分
//		target.draw(m_center, states);
//		target.draw(m_center2, states);
//
//	}
//
//private:
//	void updateCorners() {
//		// 设置角落的位置
//		m_corners[0].setPosition(m_position.x, m_position.y); // 左上角
//		m_corners[1].setPosition(m_position.x + m_size.x - m_radius * 2, m_position.y); // 右上角
//		m_corners[2].setPosition(m_position.x, m_position.y + m_size.y - m_radius * 2); // 左下角
//		m_corners[3].setPosition(m_position.x + m_size.x - m_radius * 2, m_position.y + m_size.y - m_radius * 2); // 右下角
//	}
//
//	sf::Vector2f m_size;
//	float m_radius;
//	sf::Vector2f m_position; // 存储位置
//	sf::Color m_fillColor; // 填充颜色
//	sf::Color m_outlineColor; // 边缘颜色
//	float m_outlineThickness; // 边缘厚度
//	sf::RectangleShape m_center;
//	sf::RectangleShape m_center2;
//	std::vector<sf::CircleShape> m_corners;
//};
//
//class CircleRectangle : public sf::Drawable {
//public:
//	CircleRectangle(sf::Vector2f size, float radius)
//		: round(size, radius), fill(size, radius) {};
//
//	void setPosition(float x, float y) {
//		round.setPosition(x, y);
//		fill.setPosition(x, y);
//	}
//
//	void setSize(sf::Vector2f size, float radius) {
//		round.setSize(size, radius);
//		fill.setSize(size, radius);
//	}
//
//	void setFillColor(const sf::Color& color) {
//		round.setFillColor(color);
//		fill.setFillColor(color);
//	}
//
//	void setOutlineColor(const sf::Color& color) {
//		round.setOutlineColor(color);
//	}
//
//	void setOutlineThickness(float thickness) {
//		round.setOutlineThickness(thickness);
//	}
//
//	sf::Vector2f getPosition() const {
//		return round.getPosition();
//	}
//
//	sf::FloatRect getGlobalBounds() const {
//		return round.getGlobalBounds();
//	}
//
//	sf::Vector2f getSize() const {
//		return round.getSize();
//	}
//
//	void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
//		target.draw(round);
//		target.draw(fill);
//	}
//
//private:
//	RoundedRectangle round;
//	RoundedRectangle fill;
//};
//
//int main() {
//  sf::RenderWindow window(sf::VideoMode(800, 600), "Rounded Rectangle Example");
//  window.setFramerateLimit(10);
//  // 创建圆角矩形，设置填充颜色和边缘颜色
//  //alllink::CircleRectangle roundedRect(sf::Vector2f(100, 50), 20);
//  //alllink::CircleRectangle roundedRect;
//	//roundedRect.setSize(sf::Vector2f(300, 150), 18);
//  //roundedRect.setPosition(250, 200); // 设置位置
//  //roundedRect.setFillColor(sf::Color::White);
//  //roundedRect.setOutlineColor(sf::Color::Red);
//  //roundedRect.setOutlineThickness(5);
//
//	std::string font2File = "./resources/fonts/msyh.ttc";
//	alllink::TextFillRectangle roundedRect(sf::Color::Red, 5);
//	roundedRect.init(190, 50, 250, 200, 20.0);
//	roundedRect.setColor(sf::Color::White, sf::Color::Red, sf::Color(161, 19, 19));
//	roundedRect.setText(font2File, L"离开会议", sf::Color::Black, sf::Color::White);
//  int i = 1;
//  while (window.isOpen()) {
//    sf::Event event;
//    while (window.pollEvent(event)) {
//			if (event.type == sf::Event::Closed) {
//        window.close();
//				break;
//			}
//			sf::Vector2i mousePosWin = sf::Mouse::getPosition(window);
//			sf::Vector2f mousePosView = window.mapPixelToCoords(mousePosWin);
//			if (roundedRect.onClick(event, mousePosView, &window)) {
//				I_LOG("离开会议");
//			}
//    }
//    //if(i < 490) roundedRect.setPosition(++i, 100);
//		if (i < 490) {
//			i++;
//			//roundedRect.setSize(sf::Vector2f(300 + i, 200), 20);
//		}
//    window.clear();
//    //window.draw(roundedRect);
//		roundedRect.render(&window);
//    window.display();
//  }
//
//  return 0;
//}

#include <SFML/Graphics.hpp>
#include <vector>
#include <array>
#include <cmath>

class AdvancedRoundedRectangle : public sf::Drawable {
public:
  explicit AdvancedRoundedRectangle(
    const sf::Vector2f& size = { 100.f, 50.f },
    float radius = 15.f,
    unsigned int cornerPoints = 20
  ) : m_size(size), m_cornerPoints(cornerPoints) {
    setCornerRadius(radius); // 通过setter初始化以保证约束
    updateGeometry();
  }

  void setSize(const sf::Vector2f& size) {
    m_size = size;
    m_radius = getValidRadius(); // 自动更新有效半径
    updateGeometry();
  }

  void setCornerRadius(float radius) {
    m_radius = std::clamp(radius, 0.f, getMaxRadius());
    updateGeometry();
  }

  void setPosition(const sf::Vector2f& position) {
    m_position = position;
    updateGeometry();
  }

  void setFillColor(const sf::Color& color) {
    m_fillColor = color;
    updateColors();
  }

  void setOutlineColor(const sf::Color& color) {
    m_outlineColor = color;
    updateColors();
  }

  void setOutlineThickness(float thickness) {
    m_outlineThickness = std::max(0.f, thickness);
    updateGeometry();
  }

  // 其他getter保持相同...

private:
  void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
    target.draw(m_fillVertices, states);
    if (m_outlineThickness > 0) {
      target.draw(m_outlineVertices, states);
    }
  }

  float getMaxRadius() const {
    return std::min(m_size.x, m_size.y) / 2;
  }

  float getValidRadius() const {
    return std::min(m_radius, getMaxRadius());
  }

  void updateGeometry() {
    m_fillVertices.clear();
    m_outlineVertices.clear();

    createFillGeometry();
    if (m_outlineThickness > 0) {
      createOutlineGeometry();
    }
    updateColors();
  }

  void createFillGeometry() {
    const float radius = getValidRadius();
    const float diameter = 2 * radius;
    const sf::Vector2f innerSize = m_size - sf::Vector2f(diameter, diameter);

    const std::array<sf::Vector2f, 4> centers = {
        m_position + sf::Vector2f(radius, radius),
        m_position + sf::Vector2f(m_size.x - radius, radius),
        m_position + sf::Vector2f(m_size.x - radius, m_size.y - radius),
        m_position + sf::Vector2f(radius, m_size.y - radius)
    };

    const std::array<float, 4> angles = { 180.f, 270.f, 0.f, 90.f };

    // 生成四个圆角
    for (int i = 0; i < 4; ++i) {
      createCornerFan(centers[i], angles[i], 90.f, m_fillVertices);
    }

    // 生成中间区域
    createQuad(m_position + sf::Vector2f(radius, 0), { innerSize.x, m_size.y }, m_fillVertices);
    createQuad(m_position + sf::Vector2f(0, radius), { m_size.x, innerSize.y }, m_fillVertices);
    createQuad(m_position + sf::Vector2f(radius, radius), innerSize, m_fillVertices);
  }

  void createCornerFan(const sf::Vector2f& center, float startAngle, float sweep, sf::VertexArray& target) {
    const float deg2rad = 3.14159265f / 180.f;
    const float step = sweep / (m_cornerPoints - 1);
    const float radius = getValidRadius();

    std::vector<sf::Vertex> fan;
    fan.emplace_back(center, m_fillColor);

    for (unsigned i = 0; i < m_cornerPoints; ++i) {
      float angle = (startAngle + i * step) * deg2rad;
      sf::Vector2f offset(std::cos(angle) * radius, std::sin(angle) * radius);
      fan.emplace_back(center + offset, m_fillColor);
    }

    for (size_t i = 1; i < fan.size() - 1; ++i) {
      target.append(fan[0]);
      target.append(fan[i + 1]);
      target.append(fan[i]);
    }
  }

  void createOutlineGeometry() {
    const float radius = getValidRadius();
    const float outerRadius = radius + m_outlineThickness;
    const sf::Vector2f outerSize = m_size + 2.f * sf::Vector2f(m_outlineThickness, m_outlineThickness);

    // 外边框约束
    const float maxOuterRadius = std::min(outerSize.x, outerSize.y) / 2;
    const float actualOuterRadius = std::min(outerRadius, maxOuterRadius);

    const sf::FloatRect outerRect(
      m_position.x - m_outlineThickness,
      m_position.y - m_outlineThickness,
      outerSize.x,
      outerSize.y
    );

    const sf::FloatRect innerRect(m_position, m_size);

    // 外轮廓中心
    const std::array<sf::Vector2f, 4> outerCenters = {
        sf::Vector2f(outerRect.left + actualOuterRadius, outerRect.top + actualOuterRadius),
        sf::Vector2f(outerRect.left + outerRect.width - actualOuterRadius, outerRect.top + actualOuterRadius),
        sf::Vector2f(outerRect.left + outerRect.width - actualOuterRadius, outerRect.top + outerRect.height - actualOuterRadius),
        sf::Vector2f(outerRect.left + actualOuterRadius, outerRect.top + outerRect.height - actualOuterRadius)
    };

    // 内轮廓中心
    const std::array<sf::Vector2f, 4> innerCenters = {
        sf::Vector2f(innerRect.left + radius, innerRect.top + radius),
        sf::Vector2f(innerRect.left + innerRect.width - radius, innerRect.top + radius),
        sf::Vector2f(innerRect.left + innerRect.width - radius, innerRect.top + innerRect.height - radius),
        sf::Vector2f(innerRect.left + radius, innerRect.top + innerRect.height - radius)
    };

    // 生成圆角边框
    const std::array<float, 4> angles = { 180.f, 270.f, 0.f, 90.f };
    for (int i = 0; i < 4; ++i) {
      createOutlineSegment(
        outerCenters[i],
        innerCenters[i],
        angles[i],
        90.f,
        actualOuterRadius,
        radius,
        m_outlineVertices
      );
    }

    // 生成直线边框
    createOutlineLine(
      { outerRect.left + actualOuterRadius, outerRect.top },
      { outerRect.left + outerRect.width - actualOuterRadius, outerRect.top },
      { innerRect.left + radius, innerRect.top },
      { innerRect.left + innerRect.width - radius, innerRect.top },
      m_outlineVertices
    );

    createOutlineLine(
      { outerRect.left + outerRect.width, outerRect.top + actualOuterRadius },
      { outerRect.left + outerRect.width, outerRect.top + outerRect.height - actualOuterRadius },
      { innerRect.left + innerRect.width, innerRect.top + radius },
      { innerRect.left + innerRect.width, innerRect.top + innerRect.height - radius },
      m_outlineVertices
    );

    createOutlineLine(
      { outerRect.left + actualOuterRadius, outerRect.top + outerRect.height },
      { outerRect.left + outerRect.width - actualOuterRadius, outerRect.top + outerRect.height },
      { innerRect.left + radius, innerRect.top + innerRect.height },
      { innerRect.left + innerRect.width - radius, innerRect.top + innerRect.height },
      m_outlineVertices
    );

    createOutlineLine(
      { outerRect.left, outerRect.top + actualOuterRadius },
      { outerRect.left, outerRect.top + outerRect.height - actualOuterRadius },
      { innerRect.left, innerRect.top + radius },
      { innerRect.left, innerRect.top + innerRect.height - radius },
      m_outlineVertices
    );
  }

  void createOutlineSegment(const sf::Vector2f& outerCenter, const sf::Vector2f& innerCenter,
    float startAngle, float sweep, float outerRadius, float innerRadius,
    sf::VertexArray& target) {
    const float deg2rad = 3.14159265f / 180.f;
    const float step = sweep / (m_cornerPoints - 1);

    std::vector<sf::Vertex> outerArc, innerArc;

    for (unsigned i = 0; i < m_cornerPoints; ++i) {
      float angle = (startAngle + i * step) * deg2rad;

      sf::Vector2f outerPoint = outerCenter + sf::Vector2f(
        std::cos(angle) * outerRadius,
        std::sin(angle) * outerRadius
      );

      sf::Vector2f innerPoint = innerCenter + sf::Vector2f(
        std::cos(angle) * innerRadius,
        std::sin(angle) * innerRadius
      );

      outerArc.emplace_back(outerPoint, m_outlineColor);
      innerArc.emplace_back(innerPoint, m_outlineColor);
    }

    for (size_t i = 0; i < outerArc.size() - 1; ++i) {
      target.append(outerArc[i]);
      target.append(innerArc[i]);
      target.append(outerArc[i + 1]);

      target.append(innerArc[i]);
      target.append(innerArc[i + 1]);
      target.append(outerArc[i + 1]);
    }
  }

  void createOutlineLine(const sf::Vector2f& outerStart, const sf::Vector2f& outerEnd,
    const sf::Vector2f& innerStart, const sf::Vector2f& innerEnd,
    sf::VertexArray& target) {
    // 四边形分解为两个三角形
    target.append(sf::Vertex(outerStart, m_outlineColor));
    target.append(sf::Vertex(innerStart, m_outlineColor));
    target.append(sf::Vertex(innerEnd, m_outlineColor));

    target.append(sf::Vertex(outerStart, m_outlineColor));
    target.append(sf::Vertex(innerEnd, m_outlineColor));
    target.append(sf::Vertex(outerEnd, m_outlineColor));
  }

  void createQuad(const sf::Vector2f& pos, const sf::Vector2f& size, sf::VertexArray& target) {
    const std::array<sf::Vector2f, 4> points = {
        pos,
        pos + sf::Vector2f(size.x, 0.f),
        pos + size,
        pos + sf::Vector2f(0.f, size.y)
    };

    target.append(sf::Vertex(points[0], m_fillColor));
    target.append(sf::Vertex(points[3], m_fillColor));
    target.append(sf::Vertex(points[2], m_fillColor));

    target.append(sf::Vertex(points[0], m_fillColor));
    target.append(sf::Vertex(points[2], m_fillColor));
    target.append(sf::Vertex(points[1], m_fillColor));
  }

  void updateColors() {
    for (size_t i = 0; i < m_fillVertices.getVertexCount(); ++i) {
      m_fillVertices[i].color = m_fillColor;
    }
    for (size_t i = 0; i < m_outlineVertices.getVertexCount(); ++i) {
      m_outlineVertices[i].color = m_outlineColor;
    }
  }

private:
  sf::Vector2f m_size;
  sf::Vector2f m_position;
  float m_radius = 0.f;
  unsigned int m_cornerPoints = 20;
  float m_outlineThickness = 0.f;
  sf::Color m_fillColor = sf::Color::White;
  sf::Color m_outlineColor = sf::Color::Transparent;

  sf::VertexArray m_fillVertices{ sf::Triangles };
  sf::VertexArray m_outlineVertices{ sf::Triangles };
};

int main() {
  sf::RenderWindow window(sf::VideoMode(800, 600), "Rounded Rectangle Demo");
  AdvancedRoundedRectangle rect({350.f, 10.f}, 60.f, 40);

  //rect.setSize({ 350.f, 150.f });
  //rect.setCornerRadius(40.f);
  rect.setPosition({ 225.f, 225.f });
  rect.setFillColor(sf::Color(100, 200, 255));
  rect.setOutlineColor(sf::Color::Black);
  rect.setOutlineThickness(2.f);

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
    }

    window.clear(sf::Color(240, 240, 240));
    window.draw(rect);
    window.display();
  }

  return 0;
}