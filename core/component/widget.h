#pragma once
#include "module.h"

namespace alllink {
	class BaseWidget : public sf::RenderTexture {
	public:
		BaseWidget(int width, int height, int x, int y) {
			this->create(width, height);
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

		void setInputBox(const std::string& fontFile, const sf::String& defaultText = L"请输入文本", sf::Color color = sf::Color::Black) {
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
				if ((event.type == sf::Event::MouseButtonPressed
					&& event.key.code == sf::Mouse::Left) ||
					(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)) {
					inputBox.setActive(false);
				}
			}
			inputBox.eventProcess(event);
			return isClick;
		}

		void render(sf::RenderTarget* tar) {
			this->clear(sf::Color(111, 111, 111, 0));
			this->draw(description);
			inputBox.render(this);
			this->display();
			sf::Sprite sprite(this->getTexture());
			sprite.setPosition(x_, y_);
			tar->draw(sprite);
		}

		std::string getInput() { return inputBox.getEnterText(); }

		bool empty() const { return inputBox.inputEmpty(); }

		void resetInput() { inputBox.resetText(); }

		bool getInputActive() const { return inputBox.getActive(); }

		void setInputActive(bool val) { inputBox.setActive(val); }

		void setInputVal(const std::string& val) { inputBox.setInputVal(val); }

	protected:
		InputBoxModule inputBox;
		BaseText description;
	private:
		int w, h, x, y;
	};

	class DropDescriptionWidget : public BaseWidget {
	public:
		DropDescriptionWidget(int width, int height, int x, int y, int labelNum, const std::string& fontFile)
			: BaseWidget(width, height, x, y), fontFile_(fontFile), 
			w(width_ - 4), h(height_ - 4),
			x(2), y(2), labelSize(labelNum) {
			rect = std::make_unique<ClickTextRoundRectangle>(sf::Color(5, 5, 5), 0);
			description.init(fontFile_);
		};

		void setDescription(const sf::String& text, int textSize, sf::Color color = sf::Color::Black) {
			description.setCharacterSize(textSize);
			description.setFillColor(color);
			description.setPosition(x, y + 2);
			description.setString(text);
			int rectW = w - 4;
			int rectH = h / (labelSize + 2) - 2;
			rect->init(rectW, rectH, this->x + 2, this->y + textSize + 8, 6.f);
		}

		void setTextButton(const sf::String& text, sf::Color textColor) {
			defaultDescription = text;
			rect->setText(fontFile_, text, textColor);
			rect->setColor(sf::Color(199, 199, 199), sf::Color(225, 225, 225, 200), sf::Color(225, 225, 225));
		}

		void setDropList() {
			int labelHeight = h / (labelSize + 2) - 2;
			dropList.init(w, labelSize * (labelHeight + 4), x, rect->getPosition().y + rect->getSize().y + 2, labelHeight, fontFile_);
			dropList.setFillColor(sf::Color(220, 220, 220));
		}

		void addLabel(sf::String labelText, sf::Color fillColor, sf::Color hoverColor,
			sf::Color pressColor, bool activate = true) {
			dropList.addLabel(labelText, fillColor, hoverColor, pressColor, activate);
		}

		void reset() {
			dropList.setShow(false);
			rect->setDescription(defaultDescription);
		}

		std::wstring getSelectedLabel() { return rect->getDescription(); }

		bool eventProcess(sf::Event event, sf::RenderWindow* win) {
			bool isClick = false;
			sf::Vector2i mousePosWin = sf::Mouse::getPosition(*win);
			sf::Vector2f mouseWindowPos = win->mapPixelToCoords(mousePosWin);
			sf::Vector2f mousePosView(
				mouseWindowPos.x - x_,
				mouseWindowPos.y - y_
			);
			if (rect->onClick(event, mousePosView, win)) {
				dropList.setShow(true);
			}
			else if (dropList.eventProcess(event, mousePosView, win)) {
				rect->setDescription(dropList.getSelectedLabel());
				isClick = true;
			}
			else if (event.type == sf::Event::MouseButtonReleased && event.key.code == sf::Mouse::Left) {
				dropList.setShow(false);
			}
			return isClick;
		}

		void render(sf::RenderTarget* tar) {
			this->clear(sf::Color(111, 111, 111, 0));
			this->draw(description);
			rect->render(this);
			dropList.render(this);
			this->display();
			sf::Sprite sprite(this->getTexture());
			sprite.setPosition(x_, y_);
			tar->draw(sprite);
		}

	protected:
		std::string fontFile_;
		BaseText description;
		std::unique_ptr<ClickTextRoundRectangle> rect;
		DropListModule dropList;
		sf::String defaultDescription;

	private:
		int w, h, x, y;
		int labelSize;
	};
}