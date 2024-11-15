#pragma once
#include "basescreen.h"
#include "config/path.h"
#include "component/module.h"
#include "message.h"

#include "seeker/common.h"

namespace alllink {

	enum class MessageType : int {
		/*登录窗口消息载体*/

		CREATE_MEETING = 0,
		JOIN_MEETING,
		START_LOGIN,
		START_LOGOUT,
		SETTING


	};

	static constexpr int msgTo(MessageType msg) { return static_cast<int>(msg); }

	class LoginScreen : public BaseScreen {
	public:
		enum class LoginType {
			OFFLINE = 0,
			ONLINE
		};

		LoginScreen(sf::VideoMode mode, const sf::String& title, sf::Image icon,
			sf::Uint32 style = sf::Style::Default,
			const sf::ContextSettings& settings = sf::ContextSettings());

		~LoginScreen();

		std::shared_ptr<BaseScreen> Next() override;

		std::shared_ptr<BaseScreen> Last() override;

		void OnEnter() override;

		void OnExit() override;

		int init() override;

		void show() override;

		void eventProcess() override;

		LoginType type() const;

	private:
		std::unique_ptr<VerticalGraphicTextsModule> createMeeting;
		std::unique_ptr<VerticalGraphicTextsModule> joinMeeting;
		std::unique_ptr<HorizonGraphicTextsModule> startLogin;
		std::unique_ptr<HorizonGraphicTextsModule> isLogin;
		std::unique_ptr<VariableStateGraphicModule> setting;
		BaseText todayDate;
		BaseText useId;
		LoginType type_{ LoginType::OFFLINE };
		sf::RectangleShape taskSide;
	};


}