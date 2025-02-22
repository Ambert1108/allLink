#include "screen/screensink.h"

namespace alllink {
	EnterScreen::EnterScreen(sf::VideoMode mode, const sf::String& title,
		sf::Image icon, int style) 
		: CustomScreen(mode, title, icon, style) {
		wr = static_cast<float>(mode.width) / 356;
		hr = static_cast<float>(mode.height) / 562;
		wndPosition = this->getPosition();
		this->icon_ = icon;
		this->setFramerateLimit(60);
		this->setVisible(false);
		inputMeetingIdWidget = nullptr;
		createButton = nullptr;
		joinButton = nullptr;
	}

	EnterScreen::~EnterScreen() {}

	bool EnterScreen::OnEnter() {
		if (isActive) return false;
		this->setVisible(true);
		this->setPosition(wndPosition);
		isActive = true;
		reset();
		return true;
	}

	bool EnterScreen::OnExit() {
		if (!isActive) return false;
		this->setVisible(false);
		isActive = false;
		reset();
		return true;
	}

	int EnterScreen::init() {
		inputMeetingIdWidget = std::make_unique<EnterDescriptionWidget>(290 * wr, 94 * hr, 36 * wr, 126 * hr);
		inputMeetingIdWidget->setInputBox(msyhFile, L"请输入会议号 xxx-xxx");
		inputMeetingIdWidget->setDescription(msyhFile, L"会议号");

		createButton = std::make_unique<TextRectangle>();
		createButton->init(188 * wr, 48 * hr, 84 * wr, 462 * hr);
		createButton->setText(msyhFile, L"创建会议", sf::Color::White);
		createButton->setStateColor(sf::Color(143, 170, 220), sf::Color(218, 227, 243));

		joinButton = std::make_unique<TextRectangle>();
		joinButton->init(188 * wr, 48 * hr, 84 * wr, 462 * hr);
		joinButton->setText(msyhFile, L"加入会议", sf::Color::White);
		joinButton->setStateColor(sf::Color(143, 170, 220), sf::Color(218, 227, 243));

		screenDescriptionText.init(msyhbdFile);
		screenDescriptionText.setCharacterSize(17 * hr);
		screenDescriptionText.setString(L"加入会议");
		screenDescriptionText.setFillColor(sf::Color::Black);
		screenDescriptionText.setPosition(
			(this->getSize().x - screenDescriptionText.getGlobalBounds().width) / 2,
			52);
		return 0;
	}

	void EnterScreen::show() {
		if (!isActive) return;
		this->clear(sf::Color(242, 242, 242));
		switch (style_) {
		case CustomScreen::All:
			this->square.render(this);
			[[fallthrough]];
		case CustomScreen::Minisize:
			this->horizontalLine.render(this);
			[[fallthrough]];
		case CustomScreen::Close:
			this->cross.render(this);
			break;
		}
		inputMeetingIdWidget->render(this);
		if (type_ == EnterType::CREATE) {
			this->draw(screenDescriptionText);
			createButton->render(this);
		}
		else {
			this->draw(screenDescriptionText);
			joinButton->render(this);
		}
		this->display();
	}

	void EnterScreen::eventProcess() {
		if (!isActive) return;
		while (this->pollEvent(event)) {
			this->checkStatus(event);
			inputMeetingIdWidget->eventProcess(event, this);
			if (type_ == EnterType::CREATE) {
				if ((createButton->onClick(event, getMousePosition(), this)
					|| (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter))
					&& createButton->getActive()) {
					hi::PostMsg({ msgTo(MessageType::IS_ENTER), inputMeetingIdWidget->getInput() });
					if (type_ == EnterType::CREATE) createButton->setActive(false);
					else createButton->setActive(false);
				}
			}
			else{
				if ((joinButton->onClick(event, getMousePosition(), this)
					|| (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter))
					&& joinButton->getActive()) {
					hi::PostMsg({ msgTo(MessageType::IS_ENTER), inputMeetingIdWidget->getInput() });
					if (type_ == EnterType::CREATE) createButton->setActive(false);
					else joinButton->setActive(false);
				}
			}
			if (!inputMeetingIdWidget->empty()) {
				if (type_ == EnterType::CREATE) createButton->setActive(true);
				else joinButton->setActive(true);
			}
			else {
				if (type_ == EnterType::CREATE) createButton->setActive(false);
				else joinButton->setActive(false);
			}
		}
	}

	void EnterScreen::OnFailed() {

	}

	void EnterScreen::setType(EnterType type) { 
		type_ = type;
		if (type_ == EnterType::CREATE) screenDescriptionText.setString(L"创建会议");
		else if (type_ == EnterType::JOIN) screenDescriptionText.setString(L"加入会议");
	}

	void EnterScreen::reset() {
		inputMeetingIdWidget->resetInput();
	}
}