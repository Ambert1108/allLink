#include "screen/screensink.h"
#include <regex>

namespace alllink {
	StartScreen::StartScreen(sf::VideoMode mode, const sf::String& title, sf::Image icon, int style)
		: CustomScreen(mode, title, icon, style) {
		wr = static_cast<float>(mode.width) / 640;
		hr = static_cast<float>(mode.height) / 480;
		wndPosition = this->getPosition();
		this->icon_ = icon;
		this->setFramerateLimit(60);
		this->setVisible(false);
		createMeeting = nullptr;
		joinMeeting = nullptr;
		startLogin = nullptr;
	}

	StartScreen::~StartScreen() { }


	bool StartScreen::OnEnter() {
		//TODO:设置界面可见
		this->setVisible(true);
		this->setPosition(wndPosition);
		isActive = true;
		return true;
	}


	bool StartScreen::OnExit() {
		//TODO:设置界面不可见
		this->setVisible(false);
		isActive = false;
		return true;
	}


	int StartScreen::init() {
		createMeeting = std::make_unique<VerticalGraphicTextsModule>();
		joinMeeting = std::make_unique<VerticalGraphicTextsModule>();
		startLogin = std::make_unique<HorizonGraphicTextsModule>();
		isLogin = std::make_unique<HorizonGraphicTextsModule>();

		createMeeting->init(138 * wr, 130 * hr, 425 * wr, 65 * hr);
		createMeeting->setSource(15 * hr, fzchFile, createMeetingFile);
		createMeeting->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
		createMeeting->setText(L"创建会议", sf::Color(0, 0, 0));
		createMeeting->setImageSize(72 * wr, 72 * wr);
		createMeeting->setImageColor(sf::Color(124, 171, 214));

		joinMeeting->init(138 * wr, 130 * hr, 425 * wr, 275 * hr);
		joinMeeting->setSource(15 * hr, fzchFile, joinMeetingFile);
		joinMeeting->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
		joinMeeting->setText(L"加入会议", sf::Color(0, 0, 0));
		joinMeeting->setImageSize(72 * wr, 72 * wr);
		joinMeeting->setImageColor(sf::Color(124, 171, 214));

		startLogin->init(140 * wr, 42 * hr, 95 * wr, 211 * hr);
		startLogin->setSource(20 * hr, msyhFile, startLoginFile);
		startLogin->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
		startLogin->setText(L"请登录", sf::Color(0, 0, 0));
		startLogin->setImageSize(40 * wr, 40 * wr);
		startLogin->setFill(false);

		isLogin->init(150 * wr, 42 * hr, 95 * wr, 211 * hr);
		isLogin->setSource(20 * hr, msyhFile, isLoginFile);
		isLogin->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
		isLogin->setText(L"欢迎使用", sf::Color(0, 0, 0));
		isLogin->setImageSize(40 * wr, 40 * wr);
		isLogin->setFill(false);

		todayDate.init(fzchFile);
		todayDate.setCharacterSize(40 * hr);
		std::string timePoint = seeker::time::toString(seeker::time::currentTime(), "%m-%d");
		std::regex re1(R"((\d+)\.\d+)");
		std::wregex re2(L"(\\d{2})-(\\d{2})");
		std::string output = std::regex_replace(timePoint, re1, "$1");
		todayDate.setString(std::regex_replace(WstrConv.from_bytes(output), re2, L"$1月$2日"));
		todayDate.setPosition(sf::Vector2f(87 * wr, 87 * hr));
		todayDate.setFillColor(sf::Color(0, 0, 0));

		useId.init(msyhFile);
		useId.setCharacterSize(20 * hr);
		useId.setString(L"你好");
		useId.setPosition(sf::Vector2f(105 * wr, 184 * hr));
		useId.setFillColor(sf::Color(0, 0, 0));
		return 0;
	}


	void StartScreen::show() {
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
		createMeeting->render(this);
		joinMeeting->render(this);
		if (type_ == LoginType::OFFLINE) {
			startLogin->render(this);
		}
		else {
			isLogin->render(this);
		}
		this->draw(todayDate);
		this->draw(useId);
		this->display();
	}


	void StartScreen::eventProcess() {
		if (!isActive) return;
		while (this->pollEvent(event)) {
			this->checkStatus(event);

			if (createMeeting->onClick(event, getMousePosition(), this)) {
				//点击创建会议，进行响应
				hi::PostMsg({ msgTo(MessageType::CREATE_MEETING), nullptr });
			}
			else if(joinMeeting->onClick(event, getMousePosition(), this)) {
				//点击加入会议，进行响应
				hi::PostMsg({ msgTo(MessageType::JOIN_MEETING), nullptr });
			}
			else if (startLogin->onClick(event, getMousePosition(), this) && type_ == LoginType::OFFLINE) {
				/* 发送登录消息，视觉控制器处理过后将用户信息传给登录窗口 */
				hi::PostMsg({ msgTo(MessageType::START_LOGIN), nullptr });
			}
			else if (isLogin->onClick(event, getMousePosition(), this) && type_ == LoginType::ONLINE) {
				//点击注销，进行响应
				//type_ = LoginType::OFFLINE;
				//useId.setString(L"你好");
				hi::PostMsg({ msgTo(MessageType::START_LOGOUT), nullptr });
			}
		}
	}

	StartScreen::LoginType StartScreen::type() const { return type_; }

	void StartScreen::setUseId(const std::string& id) {
		useId.setString(id);
		type_ = LoginType::ONLINE;
	}
}