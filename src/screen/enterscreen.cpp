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

		videoEncDropWidget = std::make_unique<DropDescriptionWidget>(150 * wr, 130 * hr, 103 * wr, 85 * hr, 2, msyhFile);
		videoEncDropWidget->setDescription(L"视频编码格式", 12);
		videoEncDropWidget->setTextButton(L"h264", sf::Color::Black);
		videoEncDropWidget->setDropList();
		videoEncDropWidget->addLabel(L"h264", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));
		videoEncDropWidget->addLabel(L"vp9", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));
		
		audioEncDropWidget = std::make_unique<DropDescriptionWidget>(150 * wr, 130 * hr, 103 * wr, 170 * hr, 2, msyhFile);
		audioEncDropWidget->setDescription(L"音频编码格式", 12);
		audioEncDropWidget->setTextButton(L"opus", sf::Color::Black);
		audioEncDropWidget->setDropList();
		audioEncDropWidget->addLabel(L"pcma", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));
		audioEncDropWidget->addLabel(L"opus", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));
		
		videoMcuDropWidget = std::make_unique<DropDescriptionWidget>(150 * wr, 130 * hr, 103 * wr, 255 * hr, 2, msyhFile);
		videoMcuDropWidget->setDescription(L"视频mcu选择", 12);
		videoMcuDropWidget->setTextButton(L"J组", sf::Color::Black);
		videoMcuDropWidget->setDropList();
		videoMcuDropWidget->addLabel(L"J组", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));
		videoMcuDropWidget->addLabel(L"Y组", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));
		
		audioMcuDropWidget = std::make_unique<DropDescriptionWidget>(150 * wr, 130 * hr, 103 * wr, 340 * hr, 2, msyhFile);
		audioMcuDropWidget->setDescription(L"音频mcu选择", 12);
		audioMcuDropWidget->setTextButton(L"X组", sf::Color::Black);
		audioMcuDropWidget->setDropList();
		audioMcuDropWidget->addLabel(L"X组", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));
		audioMcuDropWidget->addLabel(L"L组", sf::Color(225, 225, 225), sf::Color(230, 230, 230), sf::Color(200, 200, 200));

		isBooking = std::make_unique<CheckBoxModule>();
		isBooking->init(sf::Vector2f(16 * wr, 16 * wr), sf::Vector2f(103 * wr, 420 * hr));
		isBooking->setDescribe(msyhFile, L"预定会议", 14 * hr);
		isBooking->setBox(sf::Color::White, yesFile);
		isBooking->setInteractiveColor(sf::Color(173, 173, 173), sf::Color(37, 194, 94));

		createButton = std::make_unique<TextRectangle>();
		createButton->init(188 * wr, 48 * hr, 84 * wr, 462 * hr);
		createButton->setText(msyhFile, L"创建会议", sf::Color::White);
		createButton->setStateColor(sf::Color(143, 170, 220), sf::Color(218, 227, 243));
		createButton->setActive(true);

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
		if (type_ == EnterType::CREATE) {
			this->draw(screenDescriptionText);
			createButton->render(this);
			audioMcuDropWidget->render(this);
			videoMcuDropWidget->render(this);
			audioEncDropWidget->render(this);
			videoEncDropWidget->render(this);
			isBooking->render(this);
		}
		else {
			inputMeetingIdWidget->render(this);
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
				if (videoEncDropWidget->eventProcess(event, this)) {
					I_LOG("drop choose: {}", WstrConv.to_bytes(videoEncDropWidget->getSelectedLabel()));
				}
				if (audioEncDropWidget->eventProcess(event, this)) {
					I_LOG("drop choose: {}", WstrConv.to_bytes(videoEncDropWidget->getSelectedLabel()));
				}
				if (videoMcuDropWidget->eventProcess(event, this)) {
					I_LOG("drop choose: {}", WstrConv.to_bytes(videoEncDropWidget->getSelectedLabel()));
				}
				if (audioMcuDropWidget->eventProcess(event, this)) {
					I_LOG("drop choose: {}", WstrConv.to_bytes(videoEncDropWidget->getSelectedLabel()));
				}
				sf::Vector2i mousePosWin = sf::Mouse::getPosition(*this);
				sf::Vector2f mousePosView = this->mapPixelToCoords(mousePosWin);
				if (isBooking->onClick(event, mousePosView, this)) {
					if (isBooking->data()) I_LOG("booking meeting");
				}
				if ((createButton->onClick(event, getMousePosition(), this)
					|| (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter))
					&& createButton->getActive()) {
					std::vector<std::wstring> labelList{};
					labelList.emplace_back(videoEncDropWidget->getSelectedLabel());
					labelList.emplace_back(audioEncDropWidget->getSelectedLabel());
					labelList.emplace_back(videoMcuDropWidget->getSelectedLabel());
					labelList.emplace_back(audioMcuDropWidget->getSelectedLabel());
					hi::PostMsg({ msgTo(MessageType::IS_CREATE), labelList });
				}
			}
			else{
				if ((joinButton->onClick(event, getMousePosition(), this)
					|| (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter))
					&& joinButton->getActive()) {
					hi::PostMsg({ msgTo(MessageType::IS_JOIN), inputMeetingIdWidget->getInput() });
					joinButton->setActive(false);
				}
			}
			if (!inputMeetingIdWidget->empty()) {
				if (type_ == EnterType::JOIN) joinButton->setActive(true);
			}
			else {
				joinButton->setActive(false);
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
		videoEncDropWidget->reset();
		audioEncDropWidget->reset();
		videoMcuDropWidget->reset();
		audioMcuDropWidget->reset();
	}
}