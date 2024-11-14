#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#include <deque>
#include <string>

namespace alllink {
	class WinOper {
		enum TYPE {
			WO_NONE = -1,
			WO_AUTO_LOGIN,
			WO_LOGIN,
			WO_CALL,
			WO_DROP,
			WO_CAM_ON,
			WO_CAM_OFF,
			WO_CAM_SWITCH,
			WO_MIC_ON,
			WO_MIC_OFF,
			WO_MIC_SWITCH,
			WO_SCR_ON,
			WO_SCR_OFF,
			WO_SCR_SWITCH,
			WO_UP,
			WO_DOWN,
			WO_OPEN_CONNECT,
			WO_OPEN_STATUS,
			WO_INPUT_CLICK
		};
	};

	class WinOperQueue {
	protected:
		std::deque<WinOper> operQueue;
	public:
		int getWinOper(WinOper& winOper) {
			if (operQueue.empty()) {
				return -1;
			}
			else {
				winOper = operQueue.front();
				operQueue.pop_front();
				return 0;
			}
		}
	};

	class BaseScreen : public sf::RenderWindow, public WinOperQueue {
	public:
		BaseScreen(sf::VideoMode mode, const sf::String& title, sf::Image icon, 
			sf::Uint32 style, const sf::ContextSettings& settings) 
			: sf::RenderWindow(mode, title, style, settings) {
			this->setIcon(64, 64, icon.getPixelsPtr());
		};


		virtual std::shared_ptr<BaseScreen> Next() = 0;

		virtual std::shared_ptr<BaseScreen> Last() = 0;

		virtual void OnEnter() = 0;

		virtual void OnExit() = 0;

		virtual int init() = 0;

		virtual void show() = 0;

		virtual void eventProcess() = 0;

		void setLastScreen(std::shared_ptr<BaseScreen> screen) { lastScreen_ = screen; }

		const sf::String getTitle()const { return title_; }

		void setTitle(sf::String title) { title_ = title; }

		sf::Vector2f getMousePosition() { return this->mapPixelToCoords(sf::Mouse::getPosition(*this)); }

	protected:
		virtual ~BaseScreen() {};
		sf::String title_ = "baseScreen";
		std::shared_ptr<BaseScreen> lastScreen_ = nullptr;
		sf::Event event{};
		sf::Image icon_;
	};
}