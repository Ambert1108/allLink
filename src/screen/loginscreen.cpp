#include "screen/screensink.h"

namespace alllink {
	LoginScreen::LoginScreen(sf::VideoMode mode, const sf::String& title, sf::Image icon, int style)
		: CustomScreen(mode, title, icon, style) {
		wr = static_cast<float>(mode.width) / 478;
		hr = static_cast<float>(mode.height) / 353;
		wndPosition = this->getPosition();
		this->icon_ = icon;
		this->setFramerateLimit(60);
		this->setVisible(false);
		inputSeverAddrWidget = nullptr;
		inputUserIdWidget = nullptr;
		inputUserPwdWidget = nullptr;
		loginButton = nullptr;
	}

	LoginScreen::~LoginScreen() { }

	std::shared_ptr<BaseScreen> LoginScreen::Next() {
		return nullptr;
	}

	std::shared_ptr<BaseScreen> LoginScreen::Last() {
		return nullptr;
	}

	void LoginScreen::OnEnter() {
		if (isActive) return;
		this->setVisible(true);
		this->setPosition(wndPosition);
		isActive = true;
		I_LOG("on enter");
	}

	void LoginScreen::OnExit() {
		if (!isActive) return;
		this->setVisible(false);
		isActive = false;
		I_LOG("on exit");
	}

	int LoginScreen::init() {
		inputSeverAddrWidget = std::make_unique<EnterDescriptionWidget>(263 * wr, 94 * hr, 34 * wr, 64 * hr);
		inputUserIdWidget = std::make_unique<EnterDescriptionWidget>(159 * wr, 94 * hr, 34 * wr, 168 * hr);
		inputUserPwdWidget = std::make_unique<EnterDescriptionWidget>(159 * wr, 94 * hr, 274 * wr, 168 * hr);
		loginButton = std::make_unique<TextRectangle>();

		inputSeverAddrWidget->setInput(msyhFile);
		inputSeverAddrWidget->setDescription(msyhFile, L"服务器地址");

		inputUserIdWidget->setInput(msyhFile);
		inputUserIdWidget->setDescription(msyhFile, L"用户名");

		inputUserPwdWidget->setInput(msyhFile);
		inputUserPwdWidget->setDescription(msyhFile, L"密码");

		loginButton->init(98 * wr, 48 * hr, 176 * wr, 287 * hr);
		loginButton->setText(msyhFile, L"登录", sf::Color::White);
		loginButton->setStateColor(sf::Color(143, 170, 220), sf::Color(218, 227, 243));

		screenDescriptionText.init(msyhbdFile);
		screenDescriptionText.setCharacterSize(17 * hr);
		screenDescriptionText.setString(L"用户登录");
		screenDescriptionText.setFillColor(sf::Color::Black);
		screenDescriptionText.setPosition(
			(this->getSize().x - screenDescriptionText.getGlobalBounds().width) / 2,
			52);

		return 0;
	}

	void LoginScreen::show() {
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
		this->draw(screenDescriptionText);
		inputSeverAddrWidget->render(this);
		inputUserIdWidget->render(this);
		inputUserPwdWidget->render(this);
		loginButton->render(this);
		this->display();
	}

	void LoginScreen::eventProcess() {
		if (!isActive) return;
		while (this->pollEvent(event)) {
			this->checkStatus(event);
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tab) {
				currentInputBox = (currentInputBox % 3) + 1;
				I_LOG("{}", currentInputBox);
				if (currentInputBox == 1) {
					inputSeverAddrWidget->setInputActive(true);
					inputUserPwdWidget->setInputActive(false);
				}
				else if (currentInputBox == 2) {
					inputUserIdWidget->setInputActive(true);
					inputSeverAddrWidget->setInputActive(false);
				}
				else if (currentInputBox == 3) {
					inputUserPwdWidget->setInputActive(true);
					inputUserIdWidget->setInputActive(false);
				}
			}
			else {
				if (inputSeverAddrWidget->eventProcess(event, this)) {
					currentInputBox = 1;
					inputUserIdWidget->setInputActive(false);
					inputUserPwdWidget->setInputActive(false);
				}
				if (inputUserIdWidget->eventProcess(event, this)) {
					currentInputBox = 2;
					inputSeverAddrWidget->setInputActive(false);
					inputUserPwdWidget->setInputActive(false);
				}
				if (inputUserPwdWidget->eventProcess(event, this)) {
					currentInputBox = 3;
					inputSeverAddrWidget->setInputActive(false);
					inputUserIdWidget->setInputActive(false);
				}
			}
			if ((loginButton->onClick(event, getMousePosition(), this) 
				|| (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)) 
				&& loginButton->getActive()) {
				std::vector<std::string> info{ 
					inputSeverAddrWidget->getInput(),
					inputUserIdWidget->getInput(),
					inputUserPwdWidget->getInput() };
				hi::PostMsg({ msgTo(MessageType::IS_LOGIN), info});
			}
			if (!inputSeverAddrWidget->empty()
				&& !inputUserIdWidget->empty()
				&& !inputUserPwdWidget->empty()) {
				loginButton->setActive(true);
			}
			else loginButton->setActive(false);
		}
	}
}