#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

//#include <windows.h>
#include <locale>
#include <codecvt>
#include <vector>

#include "seeker/logger.h"
#include "seeker/loggerApi.h"


namespace alllink {
	static std::wstring_convert<std::codecvt_utf8<wchar_t>> WstrConv;

	static int setCursor(sf::RenderWindow* win_, sf::Cursor::Type cursorType) {
		sf::Cursor cursor;
		if (cursor.loadFromSystem(cursorType)) {
			win_->setMouseCursor(cursor);
		}
		return 0;
	}

	static sf::Texture createColoredTexture(const sf::Texture& originalTexture, const sf::Color& color) {
		// 创建一个图像对象
		sf::Image image = originalTexture.copyToImage();

		// 遍历图像的每个像素并修改颜色
		for (unsigned int x = 0; x < image.getSize().x; ++x) {
			for (unsigned int y = 0; y < image.getSize().y; ++y) {
				sf::Color pixelColor = image.getPixel(x, y);
				// 乘以颜色值
				pixelColor.r = static_cast<sf::Uint8>(pixelColor.r * color.r / 255);
				pixelColor.g = static_cast<sf::Uint8>(pixelColor.g * color.g / 255);
				pixelColor.b = static_cast<sf::Uint8>(pixelColor.b * color.b / 255);
				image.setPixel(x, y, pixelColor);
			}
		}

		// 创建新的纹理并加载修改后的图像
		sf::Texture coloredTexture;
		coloredTexture.loadFromImage(image);
		return coloredTexture;
	}

	class BaseText : public sf::Text {
	public:
		bool init(const std::string& fontFile) {
			if (!font_.loadFromFile(fontFile)) return false;
			this->setFont(font_);
			return true;
		}

		bool empty() const { return this->getFont() == nullptr ? true : false; }

	protected:
		sf::Font font_;
	};

	class AdvancedRoundedRectangle : public sf::Drawable {
	public:
		AdvancedRoundedRectangle() {};

		explicit AdvancedRoundedRectangle(const sf::Vector2f& size, float radius,
			unsigned int cornerPoints) 
			: m_size(size), m_cornerPoints(cornerPoints) {
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


		sf::FloatRect getGlobalBounds() const {
			return {
					m_position.x - m_outlineThickness,
					m_position.y - m_outlineThickness,
					m_size.x + 2 * m_outlineThickness,
					m_size.y + 2 * m_outlineThickness
			};
		}
		
		sf::Vector2f getSize() const {
			return m_size;
		}

		sf::Vector2f getPosition() const {
			return m_position;
		}

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

	class VariableStateModule :public sf::RectangleShape {
	public:
		VariableStateModule() {
			this->setOutlineColor(sf::Color(0, 0, 0, 0));
			this->setOutlineThickness(2);
		}

		bool onClick(sf::Event& event_, sf::Vector2f mousePos_, 
			sf::RenderWindow* win_ = nullptr, 
			sf::Mouse::Button btn = sf::Mouse::Left) {
			if (!activate_) {
				return false;
			}
			bool flag = false;
			if (this->getGlobalBounds().contains(mousePos_)) {
				if (!isHover) {
					if (win_ != nullptr) {
						setCursor(win_, curType_);
					}
					if (fill_) {
						//this->setOutlineColor(hoverColor);
						this->setFillColor(hoverColor);
					}
					isHover = true;
				}
				if (event_.type == sf::Event::MouseButtonReleased 
					&& event_.key.code == btn && isPressed) {
					isPressed = false;
					if (fill_) {
						//this->setFillColor(fillColor);
						this->setFillColor(hoverColor);
					}
					flag = true;
				}
				else {
					flag = false;
				}
				if (event_.type == sf::Event::MouseButtonPressed 
					&& event_.key.code == btn) {
					if (!isPressed) {
						if (fill_) this->setFillColor(pressColor);
					}
					isPressed = true;
				}
			}
			else {
				if (isHover) {
					if (win_ != nullptr) {
						setCursor(win_, sf::Cursor::Arrow);
					}
					if (fill_) {
						//this->setOutlineColor(sf::Color(0, 0, 0, 0));
						this->setFillColor(fillColor);
					}
					isHover = false;
				}
			}
			return flag;
		}

		void setCursorType(sf::Cursor::Type cursorType) { curType_ = cursorType; }
		 
		int setColor(sf::Color fillColor_, sf::Color hoverColor_, sf::Color pressColor_) {
			fillColor = fillColor_;
			hoverColor = hoverColor_;
			pressColor = pressColor_;
			this->setFillColor(fillColor_);
			return 0;
		}

		/* 设置是否启用点击检测 */
		void setActivate(bool val) { activate_ = val; }

		/* 设置是否启用鼠标交互 */
		void setFill(bool val) { fill_ = val; }

		virtual void render(sf::RenderTarget* win_) = 0;

	protected:
		virtual ~VariableStateModule() = default;
		bool isPressed = false;
		bool isHover = false;
		sf::Color fillColor;
		sf::Color hoverColor;
		sf::Color pressColor;
		bool activate_ = true;
		bool fill_ = true;
		sf::Cursor::Type curType_ = sf::Cursor::Type::Hand;
	};

	class VariableStateRoundModule :public AdvancedRoundedRectangle {
	public:
		VariableStateRoundModule(sf::Color color, int thickness) : AdvancedRoundedRectangle(), outLineColor(color) {
			this->setOutlineColor(outLineColor);
			this->setOutlineThickness(thickness);
		}

		bool onClick(sf::Event& event_, sf::Vector2f mousePos_,
			sf::RenderWindow* win_ = nullptr,
			sf::Mouse::Button btn = sf::Mouse::Left) {
			if (!activate_) {
				return false;
			}
			bool flag = false;
			if (this->getGlobalBounds().contains(mousePos_)) {
				if (!isHover) {
					if (win_ != nullptr) {
						setCursor(win_, curType_);
					}
					if (fill_) {
						this->setFillColor(hoverColor);
					}
					isHover = true;
				}
				if (event_.type == sf::Event::MouseButtonReleased
					&& event_.key.code == btn && isPressed) {
					isPressed = false;
					if (fill_) {
						this->setFillColor(hoverColor);
					}
					flag = true;
				}
				else {
					flag = false;
				}
				if (event_.type == sf::Event::MouseButtonPressed
					&& event_.key.code == btn) {
					if (!isPressed) {
						if (fill_) {
							this->setFillColor(pressColor);
						}
					}
					isPressed = true;
				}
			}
			else {
				if (isHover) {
					if (win_ != nullptr) {
						setCursor(win_, sf::Cursor::Arrow);
					}
					if (fill_) {
						this->setFillColor(fillColor);
					}
					isHover = false;
				}
			}
			return flag;
		}

		void setCursorType(sf::Cursor::Type cursorType) { curType_ = cursorType; }

		int setColor(sf::Color fillColor_, sf::Color hoverColor_, sf::Color pressColor_) {
			fillColor = fillColor_;
			hoverColor = hoverColor_;
			pressColor = pressColor_;
			this->setFillColor(fillColor_);
			return 0;
		}

		/* 设置是否启用点击检测 */
		void setActivate(bool val) { activate_ = val; }

		/* 设置是否启用鼠标交互 */
		void setFill(bool val) { fill_ = val; }

		virtual void render(sf::RenderTarget* win_) {
			win_->draw(*this);
		}

		virtual ~VariableStateRoundModule() = default;
	protected:
		bool isPressed = false;
		bool isHover = false;
		sf::Color outLineColor;
		sf::Color fillColor;
		sf::Color hoverColor;
		sf::Color pressColor;
		bool activate_ = true;
		bool fill_ = true;
		sf::Cursor::Type curType_ = sf::Cursor::Type::Hand;
	};

	class VariableStateRectangleModule : public VariableStateModule {
	public:

		void set(int width, int height, int x, int y) {
			this->setSize(sf::Vector2f(width, height));
			this->setPosition(x, y);
		}

		void setShapePos(float x, float y) {
			rectangle_.setPosition(sf::Vector2f(x, y));
		}

		void setShapeSize(int width, int height) {
			int w = this->getSize().x;
			int h = this->getSize().y;
			rectangle_.setSize(sf::Vector2f(width, height));
			setShapePos(this->getPosition().x + (w - width) / 2, this->getPosition().y + (h - height) / 2);
		}

		void setShapeColor(sf::Color fillColor, sf::Color outlineColor, int outlineThickness) {
			rectangle_.setFillColor(fillColor);
			rectangle_.setOutlineThickness(outlineThickness);
			rectangle_.setOutlineColor(outlineColor);
		}

		void render(sf::RenderTarget* tar) {
			tar->draw(*this);
			tar->draw(rectangle_);
		}

		void resize(float wr, float hr) {
			I_LOG("pos is {}:{}", this->getPosition().x, this->getPosition().y);
			this->scale(sf::Vector2f(wr, hr));
			I_LOG("pos is {}:{}", this->getPosition().x, this->getPosition().y);
			rectangle_.scale(sf::Vector2f(wr, hr));
			setShapePos(rectangle_.getPosition().x * wr, rectangle_.getPosition().y *hr);
		}

	protected:
		sf::RectangleShape rectangle_;
	};

	class VariableStateVertxModule : public VariableStateModule {
	public:
		VariableStateVertxModule() : ver_(6) {};

		void set(int width, int height, int x, int y) {
			this->setSize(sf::Vector2f(width, height));
			this->setPosition(x, y);
		}

		void setVer(std::vector<sf::Vertex> vec) { ver_.swap(vec); }

		void render(sf::RenderTarget* tar) {
			tar->draw(*this);
			tar->draw(ver_.data(), ver_.size(), sf::Lines);
		}

	protected:
		std::vector<sf::Vertex> ver_;
	};

	class VariableStateVertxRoundModule : public VariableStateRoundModule {
	public:
		VariableStateVertxRoundModule() :
			VariableStateRoundModule(sf::Color::Transparent, 2), ver_(6) {};

		void set(int width, int height, int x, int y, float radius) {
			this->setSize(sf::Vector2f(width, height));
			this->setCornerRadius(radius);
			this->setPosition(sf::Vector2f(x, y));
		}

		void setVer(std::vector<sf::Vertex> vec) { ver_.swap(vec); }

		void render(sf::RenderTarget* tar) {
			tar->draw(*this);
			tar->draw(ver_.data(), ver_.size(), sf::Lines);
		}

	protected:
		std::vector<sf::Vertex> ver_;
	};

	class VariableStateGraphicModule : public VariableStateModule {
	public:
		VariableStateGraphicModule() : texture_(nullptr) {};

		virtual ~VariableStateGraphicModule() {
			if (texture_) {
				delete texture_;
				texture_ = nullptr;
			}
		}

		void init(int width, int height, int x, int y) {
			if (init_) return;
			texture_ = new sf::Texture();
			this->setSize(sf::Vector2f(width, height));
			this->setPosition(x, y);
			init_ = true;
		}

		void setTexture(const std::string& textureFile, bool resetRect = false) {
			texture_->loadFromFile(textureFile);
			image_.setTexture(*texture_, resetRect);
		}

		virtual void setImage() {
			int x = this->getPosition().x + 
				(this->getSize().x - image_.getGlobalBounds().width) / 2.f;
			int y = this->getPosition().y + 
				(this->getSize().y - image_.getGlobalBounds().height) / 2.f;
			image_.setPosition(x, y);
		}

		bool setImageSize(float width, float height) {
			try {
				auto size = texture_->getSize();
				image_.setOrigin(texture_->getSize().x / 2.f, texture_->getSize().y / 2.f);
				image_.setScale(width / size.x, height / size.y);
				image_.setOrigin(0, 0);
				setImage();
			}
			catch (std::exception& ex) {
				E_LOG("[VariableStateGraphicModule::setImageSize] catch exception:{}", ex.what());
				return false;
			}
			return true;
		}

		void setImageColor(sf::Color imageColor) { 
			image_.setColor(imageColor); 
		}

		void render(sf::RenderTarget* tar) {
			tar->draw(*this);
			tar->draw(image_);
		}

	protected:
		sf::Texture* texture_;
		sf::Sprite image_;
		bool init_ = false;
	};

	class VariableStateGraphicRoundModule : public VariableStateRoundModule {
	public:
		VariableStateGraphicRoundModule() 
			: VariableStateRoundModule(sf::Color::Transparent, 1.f), texture_(nullptr) {};

		virtual ~VariableStateGraphicRoundModule() {
			if (texture_) {
				delete texture_;
				texture_ = nullptr;
			}
		}

		void init(int width, int height, int x, int y, float radius) {
			if (init_) return;
			texture_ = new sf::Texture();
			this->setSize(sf::Vector2f(width, height));
			this->setCornerRadius(radius);
			this->setPosition(sf::Vector2f(x, y));
			init_ = true;
		}

		void setTexture(const std::string& textureFile, bool resetRect = false) {
			texture_->loadFromFile(textureFile);
			image_.setTexture(*texture_, resetRect);
		}

		virtual void setImage() {
			int x = this->getPosition().x +
				(this->getSize().x - image_.getGlobalBounds().width) / 2.f;
			int y = this->getPosition().y +
				(this->getSize().y - image_.getGlobalBounds().height) / 2.f;
			image_.setPosition(x, y);
		}

		bool setImageSize(float width, float height) {
			try {
				auto size = texture_->getSize();
				image_.setOrigin(texture_->getSize().x / 2.f, texture_->getSize().y / 2.f);
				image_.setScale(width / size.x, height / size.y);
				image_.setOrigin(0, 0);
				setImage();
			}
			catch (std::exception& ex) {
				E_LOG("[VariableStateGraphicModule::setImageSize] catch exception:{}", ex.what());
				return false;
			}
			return true;
		}

		void setImageColor(sf::Color imageColor) {
			image_.setColor(imageColor);
		}

		void render(sf::RenderTarget* tar) {
			tar->draw(*this);
			tar->draw(image_);
		}

	protected:
		sf::Texture* texture_;
		sf::Sprite image_;
		bool init_ = false;
	};
	
	class GraphicTextsModule : public VariableStateModule {
	public:
		GraphicTextsModule() : texture_(nullptr) {};

		virtual ~GraphicTextsModule() {
			if (texture_) {
				delete texture_;
				texture_ = nullptr;
			}
		}

		void init(int width, int height, int x, int y) {
			texture_ = new sf::Texture();
			this->setSize(sf::Vector2f(width, height));
			this->setPosition(x, y);
		}

		void setSource(int textSize, const std::string& fontFile, const std::string& textureFile, bool rect = false) {
			text_.init(fontFile);
			text_.setCharacterSize(textSize);
			texture_->loadFromFile(textureFile);
			image.setTexture(*texture_, rect);
		}

		virtual void setText(const sf::String& text, sf::Color textColor) = 0;

		virtual void setImage() = 0;

		void setImageColor(sf::Color imageColor) { image.setColor(imageColor); }

		bool setImageSize(float width, float height) {
			try {
				auto size = texture_->getSize();
				image.setOrigin(texture_->getSize().x / 2.f, texture_->getSize().y / 2.f);
				image.setScale(width / size.x, height / size.y);
				image.setOrigin(0, 0);
				setImage();
			}
			catch (std::exception& ex) {
				E_LOG("[VerticalWidget::init] catch exception:{}", ex.what());
				return false;
			}
			return true;
		}

		void render(sf::RenderTarget* tar) {
			tar->draw(*this);
			tar->draw(image);
			tar->draw(text_);
		}

	protected:
		BaseText text_;
		sf::Texture* texture_;
		sf::Sprite image;
	};

	class VerticalGraphicTextsModule : public GraphicTextsModule {
	public:
		void setText(const sf::String& text, sf::Color textColor) override {
			text_.setString(text);
			int x = this->getPosition().x + (this->getSize().x - text_.getGlobalBounds().width) / 2.f;
			int y = this->getPosition().y + this->getSize().y - text_.getGlobalBounds().height - 10;
			text_.setPosition(x, y);
			text_.setFillColor(textColor);
			text_.setOutlineColor(textColor);
		}

		void setImage() override {
			int x = this->getPosition().x + (this->getSize().x - image.getGlobalBounds().width) / 2.f;
			int y = this->getPosition().y + 10;
			image.setPosition(x, y);
		}
	};

	class HorizonGraphicTextsModule : public GraphicTextsModule {
	public:
		HorizonGraphicTextsModule(bool isReversal) : rev(isReversal) {};
		void setText(const sf::String& text, sf::Color textColor) override {
			text_.setString(text);
			int x, y;
			if (rev) {
				x = this->getPosition().x + 10;
				y = this->getPosition().y + (this->getSize().y - text_.getGlobalBounds().height) / 2.f;
			}
			else {
				x = this->getPosition().x + image.getGlobalBounds().width + 20;
				y = this->getPosition().y + (this->getSize().y - text_.getGlobalBounds().height) / 2.f;
			}
			text_.setPosition(x, y);
			text_.setFillColor(textColor);
			text_.setOutlineColor(textColor);
		}

		void setImage() override {
			int x, y;
			if (rev) {
				x = this->getPosition().x + this->getSize().x - image.getGlobalBounds().width - 10;
				y = this->getPosition().y + (this->getSize().y - image.getGlobalBounds().height) / 2.f;
			}
			else {
				x = this->getPosition().x + 10;
				y = this->getPosition().y + (this->getSize().y - image.getGlobalBounds().height) / 2.f;
			}
			image.setPosition(x, y);
		}

	private:
		bool rev; //通常图标在文字左边，如果反转则图标在文字右边
	};

	class InputBoxModule : public VariableStateModule {
	public:
		void init(int width, int height, int x, int y) {
			this->setSize(sf::Vector2f(width, height));
			this->setPosition(x, y);
		}

		void setText(const std::string& fontFile, const sf::String& defaultText = L"请输入文本", sf::Color color = sf::Color::Black) {
			inputText.init(fontFile);
			inputText.setCharacterSize(this->getSize().y  / 1.5);
			defaultDesc = defaultText;
			if (defaultDesc.isEmpty()) inputText.setString(L"你好");
			else inputText.setString(defaultText);
			inputText.setPosition(
				this->getPosition().x + 5, 
				this->getPosition().y + (this->getSize().y - inputText.getGlobalBounds().height) / 2 - 3);
			if (defaultDesc.isEmpty()) inputText.setString("");
			inputText.setFillColor(sf::Color(255, 255, 255, 150));
			textColor = color; 
			this->setFillColor(sf::Color(232, 230, 230));
			this->setOutlineThickness(2);
			this->setOutlineColor(sf::Color(200, 200, 200));
		}

		int setActive(bool active_) {
			isActive = active_;
			if (isActive) {
				if (first) {
					inputText.setString("");
					inputText.setFillColor(textColor);
					cursorPosition = 0;
					first = false;
				}
				this->setOutlineThickness(2);
				this->setOutlineColor(sf::Color(38, 138, 209));
				this->setFillColor(pressColor);
			}
			else {
				this->setOutlineThickness(2);
				this->setOutlineColor(sf::Color(200, 200, 200));
				this->setFillColor(fillColor);
			}
			return 0;
		}

		std::string getEnterText() {
			std::string out{ text };
			resetText(); 
			return out;
		}

		void setInputVal(const std::string& val) { 
			text = val;
			inputText.setString(text);
			inputText.setFillColor(sf::Color::Black);
			cursorPosition += text.size();
			first = false;
		};

		bool inputEmpty() const { return text.empty(); }

		void resetText() {
			first = true;
			inputText.setString(defaultDesc);
			inputText.setFillColor(sf::Color(255, 255, 255, 150));
			text.clear(); 
			cursorPosition = 0;
		}

		bool getActive() const { return isActive; }

		void eventProcess(sf::Event& event_) {
			if (!isActive) return;
			if (event_.type == sf::Event::TextEntered 
				&& (event_.text.unicode > 32 && event_.text.unicode <= 126)) {
				if (inputText.getGlobalBounds().width >= this->getSize().x - 20) {
					inputText.setFillColor(sf::Color(220, 20, 20));
				}
				else {
					inputText.setFillColor(sf::Color::Black);
					if (cursorPosition < text.length()) {
						text.insert(cursorPosition, 1, event_.text.unicode);
					}
					else {
						text += event_.text.unicode;
					}
					cursorPosition++;
					inputText.setString(text);
				}
			}
			if (event_.type == sf::Event::KeyPressed) {
				if (event_.key.code == sf::Keyboard::Left) {
					if (cursorPosition > 0) {
						cursorPosition--;
					}
				}
				else if (event_.key.code == sf::Keyboard::Right) {
					if (cursorPosition < text.length()) {
						cursorPosition++;
					}
				}
				else if (event_.key.code == sf::Keyboard::Backspace) {
					if (cursorPosition > 0) {
						text.erase(cursorPosition - 1, 1);
						cursorPosition--;
						inputText.setString(text);
						inputText.setFillColor(sf::Color::Black);
					}
				}
			}
		}

		bool onClick(sf::Event& event_, sf::Vector2f mousePos_,
			sf::RenderWindow* win_ = nullptr,
			sf::Mouse::Button btn = sf::Mouse::Left) {
			if (!activate_) {
				return false;
			}
			bool flag = false;
			if (this->getGlobalBounds().contains(mousePos_)) {
				if (!isHover) {
					if (win_ != nullptr) {
						setCursor(win_, sf::Cursor::Text);
					}
					if (fill_ && !isActive) {
						this->setFillColor(hoverColor);
					}
					isHover = true;
				}
				if (event_.type == sf::Event::MouseButtonReleased
					&& event_.key.code == btn && isPressed) {
					isPressed = false;
					flag = true;
				}
				else {
					flag = false;
				}
				if (event_.type == sf::Event::MouseButtonPressed
					&& event_.key.code == btn) {
					if (!isPressed) {
						setActive(true);
					}
					isPressed = true;
				}
			}
			else {
				if (isHover) {
					if (win_ != nullptr) {
						setCursor(win_, sf::Cursor::Arrow);
					}
					if(!isActive) this->setFillColor(fillColor);
					isHover = false;
				}
			}
			return flag;
		}

		void render(sf::RenderTarget* tar) {
			tar->draw(*this);
			cursorBlinkCount++;
			if (cursorBlinkCount >= 30) {
				showCursor = !showCursor;
				cursorBlinkCount = 0;
			}
			tar->draw(inputText);
			if (showCursor && isActive) {
				sf::Vertex cursor[2];
				sf::Vector2f cursorPos = inputText.findCharacterPos(cursorPosition);

				cursor[0].position = sf::Vector2f(cursorPos.x, cursorPos.y);
				cursor[1].position = sf::Vector2f(cursorPos.x, cursorPos.y + this->getSize().y - 6);

				cursor[0].color = sf::Color::Black;
				cursor[1].color = sf::Color::Black;

				tar->draw(cursor, 2, sf::Lines);
			}
		}

	protected:
		void reset() {
			// TODO: 重置输入框
			inputText.setString("");
			inputText.setFillColor(textColor);
			cursorPosition = 0;
		}

		sf::String defaultDesc;
		BaseText inputText;
		std::string text{};
		sf::Color textColor;
		int cursorPosition = 0; //光标位置
		int cursorBlinkCount = 0;
		bool showCursor = false;
		bool isActive = false;
		bool first = true;
	};

	class TextRectangle : public VariableStateModule {
	public:
		TextRectangle() : isActive(false) {};

		void init(int width, int height, int x, int y) {
			this->setSize(sf::Vector2f(width, height));
			this->setPosition(x, y);
			this->fill_ = false;
		}

		void setText(const std::string& fontFile, const sf::String& text, sf::Color textColor) {
			text_.init(fontFile);
			text_.setCharacterSize(this->getSize().y / 2.5);
			text_.setFillColor(textColor);
			text_.setString(text);
			truncateText(text_, this->getSize().x - 4);
			text_.setPosition(
				this->getPosition().x + (this->getSize().x - text_.getGlobalBounds().width) / 2,
				this->getPosition().y + (this->getSize().y - this->getSize().y / 2) / 2);
		}

		void updateText(const sf::String& text) {
			text_.setString(text);
			truncateText(text_, this->getSize().x - 4);
			text_.setPosition(
				this->getPosition().x + (this->getSize().x - text_.getGlobalBounds().width) / 2,
				this->getPosition().y + (this->getSize().y - this->getSize().y / 2) / 2);
		}

		void setStateColor(sf::Color active, sf::Color inactive) {
			activeColor = active;
			inactiveColor = inactive;
			this->setFillColor(inactiveColor);
		}

		void setActive(bool val) { 
			isActive = val;
			if (isActive) this->setFillColor(activeColor);
			else this->setFillColor(inactiveColor);
		}

		bool getActive() const { return isActive; }

		void render(sf::RenderTarget* tar) {
			tar->draw(*this);
			tar->draw(text_);
		}

	protected:
		void truncateText(sf::Text& text, float maxWidth) {
			// 如果文本的宽度超过最大宽度，进行截断
			while (text.getGlobalBounds().width > maxWidth && text.getString().getSize() > 0) {
				std::wstring currentString = text.getString();
				currentString.pop_back(); // 删除最后一个字符
				text.setString(currentString); // 更新文本
			}
		}

		BaseText text_;
		bool isActive;
		sf::Color activeColor;
		sf::Color inactiveColor;
	};

	class TextRoundRectangle : public VariableStateRoundModule {
	public:
		TextRoundRectangle(sf::Color outlineColor, int thickness = 2)
			: VariableStateRoundModule(outlineColor, thickness) {};

		void init(int width, int height, int x, int y, float radius) {
			this->setSize(sf::Vector2f(width, height));
			this->setCornerRadius(radius);
			this->setPosition(sf::Vector2f(x, y));
		}

		void setText(const std::string& fontFile, const sf::String& text, sf::Color textColor, sf::Color hoverColor) {
			text_.init(fontFile);
			text_.setCharacterSize(this->getSize().y / 2.5);
			text_.setFillColor(textColor);
			text_.setString(text);
			truncateText(text_, this->getSize().x - 4);
			text_.setPosition(
				this->getPosition().x + (this->getSize().x - text_.getGlobalBounds().width) / 2,
				this->getPosition().y + (this->getSize().y - this->getSize().y / 2) / 2);
			textColor_ = textColor;
			textHoverColor_ = hoverColor;
		}

		void setDescription(const sf::String& text) {
			text_.setString(text);
		}

		std::wstring getDescription() const { return text_.getString(); }

		bool onClick(sf::Event& event_, sf::Vector2f mousePos_,
			sf::RenderWindow* win_ = nullptr,
			sf::Mouse::Button btn = sf::Mouse::Left) {
			if (!activate_) {
				return false;
			}
			bool flag = false;
			if (this->getGlobalBounds().contains(mousePos_)) {
				if (!isHover) {
					text_.setFillColor(textHoverColor_);
					if (win_ != nullptr) {
						setCursor(win_, curType_);
					}
					if (fill_) {
						//this->setOutlineColor(hoverColor);
						this->setFillColor(hoverColor);
					}
					isHover = true;
				}
				if (event_.type == sf::Event::MouseButtonReleased
					&& event_.key.code == btn && isPressed) {
					isPressed = false;
					if (fill_) {
						//this->setFillColor(fillColor);
						this->setFillColor(hoverColor);
						this->setOutlineColor(outLineColor);
					}
					flag = true;
				}
				else {
					flag = false;
				}
				if (event_.type == sf::Event::MouseButtonPressed
					&& event_.key.code == btn) {
					if (!isPressed) {
						if (fill_) {
							this->setFillColor(pressColor);
							this->setOutlineColor(pressColor);
						}
					}
					isPressed = true;
				}
			}
			else {
				if (isHover) {
					text_.setFillColor(textColor_);
					if (win_ != nullptr) {
						setCursor(win_, sf::Cursor::Arrow);
					}
					if (fill_) {
						//this->setOutlineColor(sf::Color(0, 0, 0, 0));
						this->setFillColor(fillColor);
						this->setOutlineColor(outLineColor);
					}
					isHover = false;
				}
			}
			return flag;
		}

		void render(sf::RenderTarget* tar) {
			tar->draw(*this);
			tar->draw(text_);
		}

	protected:
		void truncateText(sf::Text& text, float maxWidth) {
			// 如果文本的宽度超过最大宽度，进行截断
			while (text.getGlobalBounds().width > maxWidth && text.getString().getSize() > 0) {
				std::wstring currentString = text.getString();
				currentString.pop_back(); // 删除最后一个字符
				text.setString(currentString); // 更新文本
			}
		}

		BaseText text_;
		sf::Color textColor_;
		sf::Color textHoverColor_;
	};

	class ClickTextRoundRectangle : public VariableStateRoundModule {
	public:
		ClickTextRoundRectangle(sf::Color outlineColor, int thickness = 2)
			: VariableStateRoundModule(outlineColor, thickness) {
		};

		void init(int width, int height, int x, int y, float radius) {
			this->setSize(sf::Vector2f(width, height));
			this->setCornerRadius(radius);
			this->setPosition(sf::Vector2f(x, y));
		}

		void setText(const std::string& fontFile, const sf::String& text, sf::Color textColor) {
			text_.init(fontFile);
			text_.setCharacterSize(this->getSize().y / 2.5);
			text_.setFillColor(textColor);
			text_.setString(text);
			truncateText(text_, this->getSize().x - 4);
			text_.setPosition(
				this->getPosition().x + (this->getSize().x - text_.getGlobalBounds().width) / 2,
				this->getPosition().y + (this->getSize().y - this->getSize().y / 2) / 2);
			textColor_ = textColor;
		}

		void setDescription(const sf::String& text) {
			text_.setString(text);
		}

		std::wstring getDescription() const { return text_.getString(); }

		void render(sf::RenderTarget* tar) {
			tar->draw(*this);
			tar->draw(text_);
		}

	protected:
		void truncateText(sf::Text& text, float maxWidth) {
			// 如果文本的宽度超过最大宽度，进行截断
			while (text.getGlobalBounds().width > maxWidth && text.getString().getSize() > 0) {
				std::wstring currentString = text.getString();
				currentString.pop_back(); // 删除最后一个字符
				text.setString(currentString); // 更新文本
			}
		}

		BaseText text_;
		sf::Color textColor_;
	};

	class VariableStateCircleModule : public sf::CircleShape {
	public:
		VariableStateCircleModule() {
			this->setOutlineColor(sf::Color(0, 0, 0, 0));
			this->setOutlineThickness(2);
		}

		void init(float radius, int x, int y, std::size_t pointCount = 30) {
			this->setRadius(radius);
			this->setOrigin(radius, radius);
			pos = sf::Vector2f(x, y);
			this->setPosition(pos);
			this->setPointCount(pointCount);

			innerRadius = radius * 0.5;
			innerCircle.setRadius(innerRadius);
			innerCircle.setOrigin(innerRadius, innerRadius);
			innerCircle.setPosition(sf::Vector2f(x, y));
			innerCircle.setPointCount(pointCount);
		}

		void setCursorType(sf::Cursor::Type cursorType) { curType_ = cursorType; }

		int setColor(sf::Color fillColor_, sf::Color innerColor_) {
			fillColor = fillColor_;
			innerColor = innerColor_;
			this->setFillColor(fillColor_);
			innerCircle.setFillColor(innerColor_);
			return 0;
		}

		void setPos(sf::Vector2f pos) {
			this->pos = pos;
			this->setPosition(pos);
			innerCircle.setPosition(pos);
		}

		void setInnerRadius(float radius) {
			innerCircle.setRadius(radius);
			innerCircle.setOrigin(radius, radius);
		}

		void render(sf::RenderTarget* win_) {
			win_->draw(*this);
			win_->draw(innerCircle);
		}

	protected:
		sf::CircleShape innerCircle;
		sf::Color fillColor;
		sf::Color innerColor;
		float innerRadius = 0.0f;
		sf::Vector2f pos;
		
		sf::Cursor::Type curType_ = sf::Cursor::Type::Hand;
	};

	class SeekBarModule {
	public:
		/**
		* 初始化拖动条，必须调用
		* size: 拖动条尺寸
		* radius: 拖动按钮半径
		* position: 拖动条位置，按钮自动居中
		* fillColor: 拖动按钮填充颜色
		* innerColor: 拖动按钮交互颜色
		* isHorizon: 拖动是否沿水平方向移动，默认为水平方向
		* defaultData: 拖动条表达的初始数据
		*/
		void init(sf::Vector2f size, float radius, sf::Vector2f position, 
			sf::Color fillColor, sf::Color innerColor, bool isHorizon = true, float defaultData = 0.5) {
			originalSize = size;
			originalRadius = radius;
			pos = originalPosition = position;
			
			useBarOffset = originalSize.x * defaultData;
			if (isHorizon) {
				defaultPosition.x = originalPosition.x + useBarOffset;
				defaultPosition.y = originalPosition.y + originalSize.y / 2;
				circle.init(originalRadius, defaultPosition.x, defaultPosition.y, 360);
				useBar.setSize(sf::Vector2f(useBarOffset, originalSize.y));
			}
			else {
				defaultPosition.x = originalPosition.x + originalSize.x / 2;
				defaultPosition.y = originalPosition.y + useBarOffset;
				circle.init(originalRadius, defaultPosition.x, defaultPosition.y, 360);
				useBar.setSize(sf::Vector2f(originalSize.x, useBarOffset));
			}
			circle.setColor(fillColor, innerColor);
			useBar.setFillColor(sf::Color(68, 118, 235));
			useBar.setPosition(originalPosition);

			unuseBar.setSize(originalSize);
			unuseBar.setFillColor(sf::Color(189, 190, 191));
			unuseBar.setPosition(originalPosition);

			this->isHorizon = isHorizon;
		}

		/**
		* 初始化拖动条数据描述，可选调用
		* fontFile: 描述所使用的字体
		* position: 描述文本位置，可传入0-3，0:右置 1:左置 2:上置 3:下置，传入错误值为0
		* characterSize: 描述文本大小
		* textColor: 描述文本颜色
		*/
		void setText(const std::string& fontFile, int position, 
			int characterSize, sf::Color textColor) {
			describe.init(fontFile);
			describe.setString(std::to_string(static_cast<int>(useBarOffset / originalSize.x * 100)));
			if (position == 3) {
				int x = originalPosition.x - originalSize.x / 2;
				int y = originalPosition.y + originalSize.y + 15;
				describe.setPosition(sf::Vector2f(x, y));
			}
			else if(position == 2) {
				int x = originalPosition.x - originalSize.x / 2;
				int y = originalPosition.y - 30;
				describe.setPosition(sf::Vector2f(x, y));
			}
			else if(position == 1) {
				int x = originalPosition.x - 30;
				int y = originalPosition.y - std::abs(originalSize.y - characterSize) / 2;
				describe.setPosition(sf::Vector2f(x, y));
			}
			else {
				int x = originalPosition.x + originalSize.x + 15;
				int y = originalPosition.y - std::abs(originalSize.y - characterSize) / 2;
				describe.setPosition(sf::Vector2f(x, y));
			}
			//describe.setPosition(position);
			describe.setCharacterSize(characterSize);
			describe.setFillColor(textColor);
		}

		void reset() {
			circle.setPos(sf::Vector2f(defaultPosition.x, defaultPosition.y));
			if (isHorizon) {
				useBar.setSize(sf::Vector2f(useBarOffset, originalSize.y));
			}
			else {
				useBar.setSize(sf::Vector2f(originalSize.x, useBarOffset));
			}
			unuseBar.setSize(originalSize);
			describe.setString(std::to_string(static_cast<int>(useBarOffset / originalSize.x * 100)));
		}

		void eventProcess(sf::Event& event_, sf::RenderWindow* win) {
			// 获取鼠标位置
			sf::Vector2f mousePos(win->mapPixelToCoords(sf::Mouse::getPosition(*win)));
			// 检查鼠标是否在圆形内
			if (circle.getGlobalBounds().contains(mousePos)) {
				// 处理鼠标按下事件
				if (event_.type == sf::Event::MouseButtonPressed) {
					if (event_.mouseButton.button == sf::Mouse::Left) {
						if (!isPressed) {
							isDragging = true; // 开始拖动
							offset = circle.getPosition() - mousePos; // 计算偏移量
							circle.setInnerRadius(originalRadius * 0.5 * 0.75); // 鼠标按住时内圆缩小为原始半径的一半
						}
						isPressed = true;
					}
				}
			}
			else if (unuseBar.getGlobalBounds().contains(mousePos)) {
				if (event_.type == sf::Event::MouseButtonPressed) {
					if (event_.mouseButton.button == sf::Mouse::Left) {
						if (!isBarPressed) {
							isDragging = true; // 开始拖动
							offset = sf::Vector2f{ 0,0 }; // 计算偏移量
						}
						isBarPressed = true;
					}
				}
			}
			// 处理鼠标释放事件
			if (event_.type == sf::Event::MouseButtonReleased) {
				if (event_.mouseButton.button == sf::Mouse::Left) {
					if (isPressed) {
						isDragging = false; // 停止拖动
						circle.setInnerRadius(originalRadius * 0.5 * 1.6); // 鼠标松开时内圆放大至原始半径的3/4
					}
					if (isBarPressed) {
						isDragging = false; // 停止拖动
					}
					isPressed = false;
					isBarPressed = false;
				}
			}
		}

		bool update(sf::RenderWindow* win) {
			bool result = false;
			// 获取鼠标位置
			sf::Vector2f mousePos(win->mapPixelToCoords(sf::Mouse::getPosition(*win)));

			// 处理鼠标沿水平方向移动事件
			if (isDragging) {
				auto newPos = mousePos + offset;
				if (isHorizon) {
					if (newPos.x > originalSize.x + originalPosition.x) {
						newPos.x = originalSize.x + originalPosition.x;
					}
					else if (newPos.x < originalPosition.x) {
						newPos.x = originalPosition.x;
					}
					if (pos.x != newPos.x) {
						result = true;
						newPos.y = pos.y;
						pos = newPos;
						int circleY = pos.y + originalSize.y / 2;
						circle.setPos(sf::Vector2f(pos.x, circleY)); // 更新圆形位置
						if (pos.x > originalPosition.x) {
							useBar.setSize(sf::Vector2f(pos.x - originalPosition.x, originalSize.y));
						}
						else if (pos.x == originalPosition.x) {
							useBar.setSize(sf::Vector2f(1, originalSize.y));
						}
						int val = (pos.x - originalPosition.x) / originalSize.x * 100;
						describe.setString(std::to_string(val));
					}
				}

				// 处理鼠标沿垂直方向移动事件
				else {
					if (newPos.y > originalSize.y + originalPosition.y) {
						newPos.y = originalSize.y + originalPosition.y;
					}
					else if (newPos.y < originalPosition.y) {
						newPos.y = originalPosition.y;
					}
					if (pos.y != newPos.y) {
						result = true;
						newPos.x = pos.x;
						pos = newPos;
						int circleX = pos.x + originalSize.x / 2;
						circle.setPos(sf::Vector2f(circleX, pos.y)); // 更新圆形位置
						if (pos.y > originalPosition.y) {
							useBar.setSize(sf::Vector2f(originalSize.x, pos.y - originalPosition.y));
						}
						else if (pos.y == originalPosition.y) {
							useBar.setSize(sf::Vector2f(originalSize.x, 1));
						}
					}
					int val = (pos.y - originalPosition.y) / originalSize.y * 100;
					describe.setString(std::to_string(val));
				}
			}
			else {
				// 检查鼠标是否在圆形内
				if (circle.getGlobalBounds().contains(mousePos)) {
					if (!isHover) {
						setCursor(win, sf::Cursor::Type::Hand);
						circle.setInnerRadius(originalRadius * 0.5 * 1.6); // 鼠标松开时内圆放大至原始半径的3/4
						isHover = true;
					}
				}
				else if (unuseBar.getGlobalBounds().contains(mousePos)) {
					if (!isHover) {
						setCursor(win, sf::Cursor::Type::Hand);
						isHover = true;
					}
				}
				else {
					if (isHover) {
						setCursor(win, sf::Cursor::Arrow);
						circle.setInnerRadius(originalRadius * 0.5); // 恢复原尺寸
						isHover = false;
					}
				}
			}
			return result;
		}

		void render(sf::RenderTarget* tar) {
			tar->draw(unuseBar);
			tar->draw(useBar);
			circle.render(tar);
			if(!describe.empty()) tar->draw(describe);
		}

		inline int data() const {
			if (isHorizon) {
				return static_cast<float>(pos.x - originalPosition.x) / originalSize.x * 100;
			}
			return static_cast<float>(pos.y - originalPosition.y) / originalSize.y * 100;
		}

	protected:
		VariableStateCircleModule circle;
		sf::RectangleShape useBar, unuseBar;
		BaseText describe;
		float originalRadius = 10;              // 存储拖动按钮初始半径
		sf::Vector2f originalSize{ 0, 0 };      // 存储拖动条初始大小
		int useBarOffset = 0;                   // 拖动条初始偏移量
		sf::Vector2f originalPosition{ 0, 0 };  // 存储拖动按钮原点位置
		sf::Vector2f defaultPosition{ 0, 0 };   // 存储拖动按钮初始位置
		sf::Vector2f pos{ 0, 0 };               // 存储拖动按钮当前位置
		bool isHorizon = true;                  // 标记按钮是否沿水平方向移动
		bool isDragging = false;                // 标记按钮是否正在拖动
		bool isPressed = false;                 // 标记按钮是否被按下
		bool isBarPressed = false;              // 拖动条是否被按下
		bool isHover = false;                   // 标记鼠标是否悬浮于按钮上
		sf::Vector2f offset;                    // 存储鼠标相对于圆心的偏移量
	};

	class VideoModule : public sf::RectangleShape {
	public:
		VideoModule() {};

		void init(int width, int height, int x, int y) {
			this->setFillColor(sf::Color(50, 50, 50));
			this->setSize(sf::Vector2f(width, height));
			this->setPosition(sf::Vector2f(x, y));
		}

		int setVideo(const sf::Texture& texture_) {
			sf::Vector2f beforeSize = source.getGlobalBounds().getSize();
			source.setTexture(texture_, true);
			sf::Vector2f afterSize = source.getGlobalBounds().getSize();
			if (beforeSize != afterSize) {
				sf::Vector2f backSize = this->getGlobalBounds().getSize() - sf::Vector2f(this->getOutlineThickness() * 2, this->getOutlineThickness() * 2);
				float backRatio = backSize.x / backSize.y;
				float afterRatio = afterSize.x / afterSize.y;
				float scale = 1;
				if (afterRatio > backRatio) {
					scale = backSize.x / afterSize.x;
				}
				else {
					scale = backSize.y / afterSize.y;
				}
				source.setScale(scale * source.getScale().x, scale * source.getScale().y);
				sf::Vector2f newSize = source.getGlobalBounds().getSize();
				sf::Vector2f newPos = this->getPosition();
				newPos += (backSize - newSize) / 2.f;
				source.setPosition(newPos);
			}
			return 0;
		}
		
		void render(sf::RenderTarget* tar) {
			tar->draw(*this);
			tar->draw(source);
		}
		
	protected:
		sf::Sprite source;
	};

	class DropListModule :public VariableStateModule {
	public:
		void init(int width, int height, int x, int y, int labelHeight, 
			const std::string& fontFile, int limit = 5,
			float edgeMargin = 2, bool rise = false) {
			this->setSize(sf::Vector2f(width, height));
			this->setPosition(x, y);
			this->fontFile = fontFile;
			this->setFillColor(sf::Color(200, 200, 200, 0));
			this->setOutlineColor(sf::Color(200, 200, 200, 0));
			this->setOutlineThickness(1);
			point.setRadius(5);
			point.setOrigin(5, 5);
			point.setFillColor(sf::Color::Red);
			point.setPointCount(360);

			btnHeight = labelHeight;
			labelLimit = limit;
			margin = edgeMargin;
			isRise = rise;
		}

		void reset() {
			isPoint = false;
			selectedLabel.clear();
			labelList.clear();
		}
		
		bool eventProcess(sf::Event& event_, sf::Vector2f mousePos_, sf::RenderWindow* win_, bool clipHide = true) {
			if (!showList) {
				return false;
			}
			int i = 2;
			for (auto& each : labelList) {
				if (each.second.onClick(event_, mousePos_, win_)) {
					selectedLabel = each.first;
					if(clipHide) switchShow();
					point.setPosition(sf::Vector2f(each.second.getPosition().x, each.second.getPosition().y));
					isPoint = true;
					return true;
				}
				i++;
			}
			return false;
		}

		void render(sf::RenderTarget* tar) {
			if (!showList) {
				return;
			}
			tar->draw(*this);
			for (auto& each : labelList) {
				each.second.render(tar);
			}
			if (isPoint) tar->draw(point);
		}

		void addLabel(sf::String labelText, sf::Color fillColor, sf::Color hoverColor, 
			sf::Color pressColor, bool activate = true) {
			if (labelList.size() >= labelLimit) {
				D_LOG("label size is more than limit {}", labelLimit);
				return;
			}
			if (labelList.find(labelText) != labelList.end()) {
				E_LOG("label {} is exist", WstrConv.to_bytes(labelText));
				return;
			}
			auto label = labelList.emplace(labelText, TextRoundRectangle(sf::Color::Transparent, 2));
			label.first->second.setActivate(activate);
			label.first->second.init(this->getGlobalBounds().getSize().x - margin * 3, btnHeight, this->getPosition().x + margin, this->getPosition().y + (labelList.size() - 1) * (btnHeight + margin * 2) + margin, 5);
			label.first->second.setText(fontFile, labelText, sf::Color::Black, sf::Color::Black);
			label.first->second.setColor(fillColor, hoverColor, pressColor);
			label.first->second.setFill(true); 
		}
		
		void clearList() {
			labelList.clear();
		}
		
		void switchShow() {
			showList = !showList;
		}
		
		void setShow(bool isShow) {
			showList = isShow;
		}
		
		const std::wstring& getSelectedLabel() { return selectedLabel; }

	protected:
		std::string fontFile;
		int textSize = 12;
		std::map<std::wstring, TextRoundRectangle> labelList;
		sf::CircleShape point;
		float btnHeight = 60;
		float btnWidth = 60;
		float margin = 2;
		std::wstring selectedLabel{};
		bool isRise = false;
		int labelLimit = 5;
		bool showList = false;
		bool isPoint = false;
	};

	class CheckBoxModule {
	public:
		CheckBoxModule() : texture_(nullptr), isCheck(false), isPress(false), isHover(false) {};

		~CheckBoxModule() {
			if (texture_) delete texture_;
		}

		void init(sf::Vector2f size, sf::Vector2f position) {
			texture_ = new sf::Texture();
			box.setSize(size);
			box.setPosition(position);
			uncheckBox.setSize(size);
			uncheckBox.setPosition(position);
		}

		void setDescribe(const std::string& fontFile, const sf::String& text, 
			int characterSize, sf::Color textColor = sf::Color::Black) {
			description.init(fontFile);
			description.setCharacterSize(characterSize);
			description.setString(text);
			description.setFillColor(textColor);
			description.setPosition(sf::Vector2f(
				box.getPosition().x + box.getSize().x + 15,
				box.getPosition().y + (description.getGlobalBounds().height - box.getSize().y) / 2));
		}

		//如果纹理文件无效，则填充checkColor
		void setBox(sf::Color uncheckColor, const std::string& textureFile = "", sf::Color checkColor = sf::Color::Green) {
			this->checkColor = checkColor;
			this->uncheckColor = uncheckColor;
			uncheckBox.setFillColor(this->uncheckColor);
			if (texture_->loadFromFile(textureFile)) {
				box.setTexture(texture_, true);
			}
			else box.setFillColor(this->checkColor);
		}

		void setInteractiveColor(sf::Color fillColor, sf::Color hoverColor) {
			fillOutlineColor = fillColor;
			hoverOutLineColor = hoverColor;
			uncheckBox.setOutlineThickness(2);
		}

		void reset() {
			uncheckBox.setFillColor(uncheckColor);
			isCheck = false;
		}

		bool onClick(sf::Event& event_, sf::Vector2f mousePos_, sf::RenderWindow* win_) {
			bool flag = false;
			if (isCheck) {
				if (!isHover) {
					if (win_ != nullptr) {
						setCursor(win_, sf::Cursor::Hand);
					}
					isHover = true;
				}
				if (box.getGlobalBounds().contains(mousePos_)) {
					if (event_.type == sf::Event::MouseButtonReleased
						&& event_.key.code == sf::Mouse::Left && isPress) {
						isPress = false;
						isCheck = false;
						flag = true;
					}
					else {
						flag = false;
					}
					if (event_.type == sf::Event::MouseButtonPressed
						&& event_.key.code == sf::Mouse::Left) {
						isPress = true;
					}
				}
				else {
					if (isHover) {
						if (win_ != nullptr) {
							setCursor(win_, sf::Cursor::Arrow);
						}
						isHover = false;
					}
				}
			}
			else {
				if (uncheckBox.getGlobalBounds().contains(mousePos_)) {
					if (!isHover) {
						if (win_ != nullptr) {
							setCursor(win_, sf::Cursor::Hand);
						}
						uncheckBox.setOutlineColor(hoverOutLineColor);
						isHover = true;
					}
					if (event_.type == sf::Event::MouseButtonReleased
						&& event_.key.code == sf::Mouse::Left && isPress) {
						isPress = false;
						isCheck = true;
						flag = true;
					}
					else {
						flag = false;
					}
					if (event_.type == sf::Event::MouseButtonPressed
						&& event_.key.code == sf::Mouse::Left) {
						isPress = true;
					}
				}
				else {
					if (isHover) {
						if (win_ != nullptr) {
							setCursor(win_, sf::Cursor::Arrow);
						}
						uncheckBox.setOutlineColor(fillOutlineColor);
						isHover = false;
					}
				}
			}
			return flag;
		}

		void render(sf::RenderTarget* tar) {
			if (isCheck) tar->draw(box);
			else tar->draw(uncheckBox);
			if(!description.empty()) tar->draw(description);
		}

		inline bool data() const { return isCheck; }
	protected:
		sf::RectangleShape box, uncheckBox;
		BaseText description;
		sf::Texture* texture_;
		sf::Color checkColor, uncheckColor;
		sf::Color fillOutlineColor, hoverOutLineColor;
		bool isCheck, isPress, isHover;
	};
}