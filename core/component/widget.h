#pragma once
#include "module.h"

namespace alllink {
	class BaseWidget : public sf::RenderTexture {
	public:
		BaseWidget(int width, int height, int x, int y) {
			this->create(width, height);
			//sf::View view(sf::FloatRect(0, 0, width, height));
			//this->setView(view);
			width_ = width;
			height_ = height;
			x_ = x;
			y_ = y;
		}

	protected:
		int width_ = 0, height_ = 0;
		int x_ = 0, y_ = 0;
	};

	class EnterDescriptionWidget : public BaseWidget {
	public:
		EnterDescriptionWidget(int width, int height, int x, int y) :
			BaseWidget(width, height, x, y), w(width_ - 4), h((height_ - 4) / 2),
			x(2), y(h) {};

		void setInput(const std::string& fontFile, const sf::String& defaultText = L"请输入文本", sf::Color color = sf::Color::Black) {
			inputBox.init(w - 5, h / 1.5, x + 5, y);
			inputBox.setText(fontFile, defaultText, color);
			inputBox.setColor(sf::Color(215, 215, 215), sf::Color(205, 205, 205), sf::Color(255, 255, 255));
		}

		void setDescription(const std::string& fontFile, const sf::String& text, sf::Color color = sf::Color::Black) {
			description.init(fontFile);
			description.setCharacterSize(h / 2.5);
			description.setFillColor(color);
			description.setPosition(x, y - h - 2);
			description.setString(text);
		}

		bool eventProcess(sf::Event event, sf::RenderWindow* win) {
			bool isClick = false;
			sf::Vector2i mousePosWin = sf::Mouse::getPosition(*win);
			sf::Vector2f mouseWindowPos = win->mapPixelToCoords(mousePosWin);
			sf::Vector2f mousePosView(
				mouseWindowPos.x - x_,
				mouseWindowPos.y - y_
			);
			if (inputBox.onClick(event, mousePosView, win)) {
				inputBox.setActive(true);
				isClick = true;
			}
			else {
				if (event.type == sf::Event::MouseButtonPressed
					&& event.key.code == sf::Mouse::Left) {
					inputBox.setActive(false);
				}
			}
			inputBox.eventProcess(event);
			return isClick;
		}

		void render(sf::RenderTarget* tar) {
			this->clear(sf::Color(126, 216, 136, 0));
			this->draw(description);
			inputBox.render(this);
			this->display();
			sf::Sprite sprite(this->getTexture());
			sprite.setPosition(x_, y_);
			tar->draw(sprite);
		}

		std::string getInput() const { return inputBox.getEnterText(); }

		bool empty() const { return inputBox.inputEmpty(); }

		bool getInputActive() const { return inputBox.getActive(); }
		void setInputActive(bool val) { inputBox.setActive(val); }

	protected:
		InputBoxMoudule inputBox;
		BaseText description;
	private:
		int w, h, x, y;
	};
}