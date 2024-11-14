#pragma once
#include "basescreen.h"
#include "config/path.h"
#include "component/module.h"

namespace alllink {
	class LoginScreen : public BaseScreen {
	public:
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

	private:
		std::unique_ptr<VerticalGraphicTextsModule> createMeeting;
		std::unique_ptr<VerticalGraphicTextsModule> joinMeeting;
		sf::Text todayDate;
		sf::Text useId;
		sf::Text useType;
	};


}