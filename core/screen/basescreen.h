#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#include <deque>
#include <string>

namespace alllink {
	class BaseScreen : public sf::RenderWindow {
	public:
		BaseScreen(sf::VideoMode mode, const sf::String& title, sf::Image icon, 
			sf::Uint32 style, const sf::ContextSettings& settings) 
			: sf::RenderWindow(mode, title, style, settings), isActive(false) {
			this->setIcon(64, 64, icon.getPixelsPtr());
		};
		 virtual ~BaseScreen() {};

		virtual bool OnEnter() = 0;

		virtual bool OnExit() = 0;

		virtual int init() = 0;

		virtual void show() = 0;

		virtual void eventProcess() = 0;

		virtual void OnFailed() = 0;

		const sf::String getTitle()const { return title_; }

		void setTitle(sf::String title) { title_ = title; }

		sf::Vector2f getMousePosition() { return this->mapPixelToCoords(sf::Mouse::getPosition(*this)); }

	protected:
		sf::String title_ = "baseScreen";
		sf::Event event{};
		sf::Image icon_;
		bool isActive;
	};
}