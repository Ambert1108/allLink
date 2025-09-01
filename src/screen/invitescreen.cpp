#include "screen/screensink.h"
#include "seeker/iniConfig.hpp"

namespace alllink {
	InviteScreen::InviteScreen(sf::VideoMode mode, const sf::String& title, sf::Image icon, int style)
		: CustomScreen(mode, title, icon, style) {
		wr = static_cast<float>(mode.width) / 478;
		hr = static_cast<float>(mode.height) / 353;
		wndPosition = this->getPosition();
		this->icon_ = icon;
		this->setFramerateLimit(60);
		this->setVisible(false);
		inputUserIdWidget = nullptr;
		inviteButton = nullptr;
	}

	InviteScreen::~InviteScreen() {}

	bool InviteScreen::OnEnter() {
		if (isActive) return false;
		this->setVisible(true);
		this->setPosition(wndPosition);
		isActive = true;
		return true;
	}

	bool InviteScreen::OnExit() {
		if (!isActive) return false;
		this->setVisible(false);
		isActive = false;
		reset();
		return true;
	}

	int InviteScreen::init() {
		inputUserIdWidget = std::make_unique<EnterDescriptionWidget>(160 * wr, 94 * hr, 158 * wr, 135 * hr);
		inviteButton = std::make_unique<TextRectangle>();

		inputUserIdWidget->setInputBox(msyhFile, L"请输入");
		//inputUserIdWidget->setInputVal(user);
		inputUserIdWidget->setDescription(msyhFile, L"用户名称");

		inviteButton->init(98 * wr, 48 * hr, 176 * wr, 287 * hr);
		inviteButton->setText(msyhFile, L"呼叫", sf::Color::White);
		inviteButton->setStateColor(sf::Color(143, 170, 220), sf::Color(218, 227, 243));

		screenDescriptionText.init(msyhbdFile);
		screenDescriptionText.setCharacterSize(17 * hr);
		screenDescriptionText.setString(L"外呼用户");
		screenDescriptionText.setFillColor(sf::Color::Black);
		screenDescriptionText.setPosition(
			(this->getSize().x - screenDescriptionText.getGlobalBounds().width) / 2,
			50);

		return 0;
	}

	void InviteScreen::show() {
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
		inputUserIdWidget->render(this);
		this->draw(screenDescriptionText);
		inviteButton->render(this);
		this->display();
	}

	void InviteScreen::eventProcess() {
		if (!isActive) return;
		while (this->pollEvent(event)) {
			this->checkStatus(event);
			(inputUserIdWidget->eventProcess(event, this));
			if ((inviteButton->onClick(event, getMousePosition(), this) 
				|| (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)) 
				&& inviteButton->getActive()) {
				
				std::vector<std::string> info{ 
					sessionId,
					inputUserIdWidget->getInput() };
				hi::PostMsg({ msgTo(MessageType::INVITE_USER), info});
				inviteButton->setActive(false);
			}
			if (!inputUserIdWidget->empty()) {
				inviteButton->setActive(true);
			}
			else inviteButton->setActive(false);
		}
	}

	void InviteScreen::OnFailed() {

	}

	void InviteScreen::reset() {
		inputUserIdWidget->resetInput();
	}

	void InviteScreen::setSessionId(const std::string& id) {
		sessionId = id;
		I_LOG("inviteScreen: sessionId is {}", sessionId);
	}
}