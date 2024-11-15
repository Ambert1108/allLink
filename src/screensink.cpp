#include "screen/screensink.h"

namespace alllink {
	LoginScreen::LoginScreen(sf::VideoMode mode, const sf::String& title, sf::Image icon,
		sf::Uint32 style, const sf::ContextSettings& settings)
		: BaseScreen(mode, title, icon, style, settings) {
		this->icon_ = icon;
		this->setFramerateLimit(60);
		this->setVisible(false);
		createMeeting = nullptr;
		joinMeeting = nullptr;
		startLogin = nullptr;
	}

	LoginScreen::~LoginScreen() {
	}

	std::shared_ptr<BaseScreen> LoginScreen::Next() { 
		return nullptr;
	}


	std::shared_ptr<BaseScreen> LoginScreen::Last() { return lastScreen_; }


	void LoginScreen::OnEnter() {
		//TODO:设置界面可见
		this->setVisible(true);
	}


	void LoginScreen::OnExit() {
		//TODO:设置界面不可见
		this->setVisible(false);
	}


	int LoginScreen::init() {
		createMeeting = std::make_unique<VerticalGraphicTextsModule>();
		joinMeeting = std::make_unique<VerticalGraphicTextsModule>();
		startLogin = std::make_unique<HorizonGraphicTextsModule>();
		isLogin = std::make_unique<HorizonGraphicTextsModule>();

		createMeeting->init(138, 130, 425, 65);
		createMeeting->setSource(15, fzchFile, createMeetingFile);
		createMeeting->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
		createMeeting->setText(L"创建会议", sf::Color(0, 0, 0));
		createMeeting->setImage();
		createMeeting->setImageColor(sf::Color(124, 171, 214));

		joinMeeting->init(138, 130, 425, 275);
		joinMeeting->setSource(15, fzchFile, joinMeetingFile);
		joinMeeting->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
		joinMeeting->setText(L"加入会议", sf::Color(0, 0, 0));
		joinMeeting->setImage();
		joinMeeting->setImageColor(sf::Color(124, 171, 214));

		startLogin->init(140, 42, 95, 211);
		startLogin->setSource(20, msyhFile, startLoginFile);
		startLogin->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
		startLogin->setText(L"请登录", sf::Color(0, 0, 0));
		startLogin->setImage();
		startLogin->setFill(false);

		isLogin->init(150, 42, 95, 211);
		isLogin->setSource(20, msyhFile, isLoginFile);
		isLogin->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
		isLogin->setText(L"欢迎使用", sf::Color(0, 0, 0));
		isLogin->setImage();
		isLogin->setFill(false);

		todayDate.init(fzchFile);
		todayDate.setCharacterSize(40);
		return 0;
	}


	void LoginScreen::show() {
		this->clear(sf::Color(240, 240, 240));
		createMeeting->render(this);
		joinMeeting->render(this);
		if (type_ == LoginScreen::LoginType::OFFLINE) {
			startLogin->render(this);
		}
		else {
			isLogin->render(this);
		}
		this->display();
	}


	void LoginScreen::eventProcess() {
		while (this->pollEvent(event)) {
			if (createMeeting->onClick(event, getMousePosition(), this)) {
				//点击创建会议，进行响应
			}
			else if(joinMeeting->onClick(event, getMousePosition(), this)) {
				//点击加入会议，进行响应
			}
			else if(startLogin->onClick(event, getMousePosition(), this)
				&& type_ == LoginScreen::LoginType::OFFLINE) {
				//点击登录，进行响应
				I_LOG("3");
				type_ = LoginScreen::LoginType::ONLINE;
			}
			else if (isLogin->onClick(event, getMousePosition(), this)
				&& type_ == LoginScreen::LoginType::ONLINE) {
				//点击注销，进行响应
				I_LOG("4");
				type_ = LoginScreen::LoginType::OFFLINE;
			}
			switch (event.type) {
			case sf::Event::Closed:
				this->close();
				break;

			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Escape) {
					this->close();
				}
				break;
			}
		}
	}

	LoginScreen::LoginType LoginScreen::type() const { return type_; }
}