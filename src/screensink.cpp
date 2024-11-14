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
		createMeeting->init1(15, fzchFile, createMeetingFile);
		createMeeting->init2(138, 130, 425, 65);
		createMeeting->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
		createMeeting->setText(L"创建会议", sf::Color(0, 0, 0));
		createMeeting->setImage();
		createMeeting->setImageColor(sf::Color(124, 171, 214));
		joinMeeting->init1(15, fzchFile, joinMeetingFile);
		joinMeeting->init2(138, 130, 425, 275);
		joinMeeting->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
		joinMeeting->setText(L"加入会议", sf::Color(0, 0, 0));
		joinMeeting->setImage();
		joinMeeting->setImageColor(sf::Color(124, 171, 214));
		return 0;
	}


	void LoginScreen::show() {
		this->clear(sf::Color(240, 240, 240));
		createMeeting->render(this);
		joinMeeting->render(this);
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
}