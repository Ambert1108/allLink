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

	class BaseText : public sf::Text {
	public:
		bool init(const std::string& fontFile) {
			if (!font_.loadFromFile(fontFile)) return false;
			this->setFont(font_);
			return true;
		}

	protected:
		sf::Font font_;
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
				E_LOG("[VerticalWidget::init] catch exception:{}", ex.what());
				return false;
			}
			return true;
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

		void setImageColor(sf::Color imageColor) { image.setColor(sf::Color(117, 188, 255)); }

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
		void setText(const sf::String& text, sf::Color textColor) override {
			text_.setString(text);
			int x = this->getPosition().x + image.getGlobalBounds().width + 20;
			int y = this->getPosition().y + (this->getSize().y - text_.getGlobalBounds().height) / 2.f;
			text_.setPosition(x, y);
			text_.setFillColor(textColor);
			text_.setOutlineColor(textColor);
		}

		void setImage() override {
			int x = this->getPosition().x + 10;
			int y = this->getPosition().y + (this->getSize().y - image.getGlobalBounds().height) / 2.f;
			image.setPosition(x, y);
		}
	};

	class InputBoxMoudule : public VariableStateModule {
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
		BaseText text_;
		bool isActive;
		sf::Color activeColor;
		sf::Color inactiveColor;
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
}