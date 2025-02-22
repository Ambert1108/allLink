#include "screen/screensink.h"
#include "seeker/iniConfig.hpp"

namespace alllink {
	LoginScreen::LoginScreen(sf::VideoMode mode, const sf::String& title, sf::Image icon, int style)
		: CustomScreen(mode, title, icon, style) {
		wr = static_cast<float>(mode.width) / 478;
		hr = static_cast<float>(mode.height) / 353;
		wndPosition = this->getPosition();
		this->icon_ = icon;
		this->setFramerateLimit(60);
		this->setVisible(false);
		serverDropWidget = nullptr;
		inputUserIdWidget = nullptr;
		inputUserPwdWidget = nullptr;
		loginButton = nullptr;
	}

	LoginScreen::~LoginScreen() {}

	bool LoginScreen::OnEnter() {
		if (isActive) return false;
		this->setVisible(true);
		this->setPosition(wndPosition);
		isActive = true;
		return true;
	}

	bool LoginScreen::OnExit() {
		if (!isActive) return false;
		this->setVisible(false);
		isActive = false;
		reset();
		return true;
	}

	int LoginScreen::init() {
		serverDropWidget = std::make_unique<DropDescriptionWidget>(263 * wr, 260 * hr, 100 * wr, 74 * hr, 4 * wr, msyhFile);
		inputUserIdWidget = std::make_unique<EnterDescriptionWidget>(159 * wr, 94 * hr, 34 * wr, 168 * hr);
		inputUserPwdWidget = std::make_unique<EnterDescriptionWidget>(159 * wr, 94 * hr, 274 * wr, 168 * hr);
		loginButton = std::make_unique<TextRectangle>();

		serverDropWidget->setDescription(L"选择信令服务器");
		serverDropWidget->setTextButton(L"J组公网信令", sf::Color::Black);
		serverDropWidget->setDropList();
		int serverNum = seeker::IniConfig::GetInteger("server", "num", 1);
		for (int i = 0; i < serverNum; i++) {
			std::string ip = seeker::IniConfig::Get("server", "signling" + std::to_string(i), "");
			serverList.emplace(i, ip);
			if (i == 0) {
				serverMap.emplace(L"J组公网信令", i);
				serverDropWidget->addLabel(L"J组公网信令", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));
			}
			else if (i == 1) {
				serverMap.emplace(L"X组公网信令", i);
				serverDropWidget->addLabel(L"X组公网信令", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));
			}
			else if (i == 2) {
				serverMap.emplace(L"J组内网信令", i);
				serverDropWidget->addLabel(L"J组内网信令", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));
			}
			else if (i == 3) {
				serverMap.emplace(L"X组内网信令", i);
				serverDropWidget->addLabel(L"X组内网信令", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));
			}
		}

		std::string user = seeker::IniConfig::Get("this", "userId", "a");
		std::string pwd = seeker::IniConfig::Get("this", "passwd", "1");

		inputUserIdWidget->setInputBox(msyhFile, "");
		inputUserIdWidget->setInputVal(user);
		inputUserIdWidget->setDescription(msyhFile, L"用户名");

		inputUserPwdWidget->setInputBox(msyhFile, "");
		inputUserPwdWidget->setInputVal(pwd);
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
		inputUserIdWidget->render(this);
		inputUserPwdWidget->render(this);
		this->draw(screenDescriptionText);
		loginButton->render(this);
		serverDropWidget->render(this);
		this->display();
	}

	void LoginScreen::eventProcess() {
		if (!isActive) return;
		while (this->pollEvent(event)) {
			this->checkStatus(event);
			if (serverDropWidget->eventProcess(event, this)) {
				I_LOG("drop choose: {}", WstrConv.to_bytes(serverDropWidget->getSelectedLabel()));
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tab) {
				currentInputBox = (currentInputBox % 2) + 1;
				if (currentInputBox == 1) {
					inputUserIdWidget->setInputActive(true);
					inputUserPwdWidget->setInputActive(false);
				}
				else if (currentInputBox == 2) {
					inputUserPwdWidget->setInputActive(true);
					inputUserIdWidget->setInputActive(false);
				}
			}
			else {
				if (inputUserIdWidget->eventProcess(event, this)) {
					currentInputBox = 1;
					inputUserPwdWidget->setInputActive(false);
				}
				if (inputUserPwdWidget->eventProcess(event, this)) {
					currentInputBox = 2;
					inputUserIdWidget->setInputActive(false);
				}
			}
			if ((loginButton->onClick(event, getMousePosition(), this) 
				|| (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)) 
				&& loginButton->getActive()) {
				sf::String label = serverDropWidget->getSelectedLabel();
				auto it = serverMap.find(label);
				if (it == serverMap.end()) {
					E_LOG("Failed find label {}", WstrConv.to_bytes(label));
				}
				auto it2 = serverList.find(it->second);
				if (it2 == serverList.end()) {
					E_LOG("Failed to find ip with id {}", it->second);
				}
				std::vector<std::string> info{ 
					it2->second,
					inputUserIdWidget->getInput(),
					inputUserPwdWidget->getInput() };
				hi::PostMsg({ msgTo(MessageType::IS_LOGIN), info});
				loginButton->setActive(false);
			}
			if (!inputUserIdWidget->empty()
				&& !inputUserPwdWidget->empty()) {
				loginButton->setActive(true);
			}
			else loginButton->setActive(false);
		}
	}

	void LoginScreen::OnFailed() {

	}

	void LoginScreen::reset() {
		serverDropWidget->reset();
		inputUserIdWidget->resetInput();
		inputUserPwdWidget->resetInput();
	}
}