#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

//#include <windows.h>

#include "seeker/logger.h"
#include "seeker/loggerApi.h"


namespace alllink {
	static int setCursor(sf::RenderWindow* win_, sf::Cursor::Type cursorType) {
		sf::Cursor cursor;
		if (cursor.loadFromSystem(cursorType)) {
			win_->setMouseCursor(cursor);
		}
		return 0;
	}

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
						setCursor(win_, sf::Cursor::Hand);
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

		int setColor(sf::Color fillColor_, sf::Color hoverColor_, sf::Color pressColor_) {
			fillColor = fillColor_;
			hoverColor = hoverColor_;
			pressColor = pressColor_;
			this->setFillColor(fillColor_);
			return 0;
		}

		void setActivate(bool val) { activate_ = val; }

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
	};

	class VerticalGraphicTextsModule : public VariableStateModule {
	public:
		VerticalGraphicTextsModule() = default;

		~VerticalGraphicTextsModule() {
			if (texture_) {
				delete texture_;
				texture_ = nullptr;
			}
		}

		void init1(int textSize, const std::string& fontFile, const std::string& textureFile) {
			font.loadFromFile(fontFile);
			text_.setFont(font);
			text_.setCharacterSize(textSize);
			texture_ = new sf::Texture();
			texture_->loadFromFile(textureFile);
			image.setTexture(*texture_);
		}

		void init2(int width, int height, int x, int y) {
			this->setSize(sf::Vector2f(width, height));
			this->setPosition(x, y);
		}

		void setText(const sf::String& text, sf::Color textColor) {
			text_.setString(text);
			int x = this->getPosition().x + (this->getSize().x - text_.getGlobalBounds().width) / 2.f;
			int y = this->getPosition().y + this->getSize().y - 20;
			text_.setPosition(x, y);
			text_.setColor(textColor);
		}

		void setImage() {
			int x = this->getPosition().x + (this->getSize().x - image.getGlobalBounds().width) / 2.f;
			int y = this->getPosition().y + 10;
			image.setPosition(x, y);
		}

		void setImageColor(sf::Color imageColor){ image.setColor(sf::Color(117, 188, 255)); }

		virtual bool setImageSize(int width, int height) {
			try {
				auto size = texture_->getSize();
				image.setOrigin(texture_->getSize().x / 2.f, texture_->getSize().y / 2.f);
				image.setScale(2, 2);
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

	private:
		sf::Font font;
		sf::Text text_;
		sf::Texture* texture_ = nullptr;
		sf::Sprite image;
	};
}