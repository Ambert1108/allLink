#pragma once

#include "SFML/OpenGL.hpp"
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include "SFML/Network.hpp"

#include <iostream>
#include <vector>
#include <ctime>
#include <sstream>

class Game {
private:
	//Variables
	//Window
	sf::RenderWindow* window;
	sf::VideoMode videoMode;
	sf::Event ev{};

	//Mouse position
	sf::Vector2i mousePosWindow;
	sf::Vector2f mousePosView;

	//Resources
	sf::Font font;

	//Text
	sf::Text uiText;

	//Game logic
	unsigned points;
	int health;
	bool endGame;
	float enemySpawnTimer;
	float enemySpawnTimerMax;
	int maxEnemies;
	bool mouseHeld;

	//Game objects
	sf::RectangleShape enemy;
	std::vector<sf::RectangleShape> enemies{};

	void initializeVariables(){
		window = nullptr;
		points = 0;
		health = 10;
		endGame = false;
		enemySpawnTimerMax = 10.0f;
		enemySpawnTimer = enemySpawnTimerMax;
		maxEnemies = 5;
		mouseHeld = false;
	}

	void initEnemies() {
		enemy.setPosition(15.f, 15.f);
		enemy.setSize(sf::Vector2f(100.f, 100.f));
		enemy.setFillColor(sf::Color::Cyan);
		enemy.setOutlineColor(sf::Color::Green);
		enemy.setOutlineThickness(5.f);
	}

	void initWindow() {
		videoMode = sf::VideoMode(1280, 720);
		window = new sf::RenderWindow(videoMode, "Game1", sf::Style::Default);
		window->setFramerateLimit(60);
	}

	void initText() {
		if (!font.loadFromFile("./resources/fonts/fzch.ttf")) {
			std::cout << "Error::initFonts: load font failed\n";
		}
		uiText.setFont(font);
		uiText.setCharacterSize(24);
		uiText.setFillColor(sf::Color::White);
		uiText.setString("None");
	}

	void spawnEnemy() {

		enemy.setPosition(
			static_cast<float>(rand() % static_cast<int>(window->getSize().x - enemy.getSize().x)),
			0.0f
		);

		int type = rand() % 4;
		switch (type) {
		case 0:
			enemy.setSize(sf::Vector2f(30.f, 30.f));
			enemy.setFillColor(sf::Color::Red);
			break;
		case 1:
			enemy.setSize(sf::Vector2f(50.f, 50.f));
			enemy.setFillColor(sf::Color::Yellow);
			break;
		case 2:
			enemy.setSize(sf::Vector2f(70.f, 70.f));
			enemy.setFillColor(sf::Color::Blue);
			break;
		default:
			enemy.setSize(sf::Vector2f(100.f, 100.f));
			enemy.setFillColor(sf::Color::Cyan);
			enemy.setOutlineColor(sf::Color::Green);
			break;
		}

		enemies.push_back(enemy);
	}

	void pollEvent() {
		//Event Polling
		while (window->pollEvent(ev)) {
			switch (ev.type) {
			case sf::Event::Closed:
				window->close();
				break;

			case sf::Event::KeyPressed:
				if (ev.key.code == sf::Keyboard::Escape)
					window->close();
				break;
			}
		}
	}

	void updateMousePositions() {
		mousePosWindow = sf::Mouse::getPosition(*window);
		mousePosView = window->mapPixelToCoords(mousePosWindow);
	}

	void updateText() {
		std::stringstream ss;
		ss << "Points:" << points << " Health:" << health;
		uiText.setString(ss.str());
	}

	void updateEnemies() {
		// Update the timer for enemy spawning
		if (enemies.size() <= maxEnemies) {
			if (enemySpawnTimer >= enemySpawnTimerMax) {
				spawnEnemy();
				enemySpawnTimer = 0.0f;
			}
			else
				enemySpawnTimer += 1.0f;
		}

		//Moveing and updating the enemies
		for (int i = 0; i < enemies.size(); i++) {
			enemies[i].move(0.0f, 5.0f);

			//If the enemy is past the bottom of the screen
			if (enemies[i].getPosition().y > window->getSize().y) {
				enemies.erase(enemies.begin() + i);
				health -= 1;
				std::cout << "Health: " << health << "\n";
			}
		}

		//Check if click upon
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			if (!mouseHeld) {
				mouseHeld = true;
				bool deleted = false;
				for (size_t i = 0; i < enemies.size() && deleted == false; i++) {

					if (enemies[i].getGlobalBounds().contains(mousePosView)) {
						//Gain poits
						if (enemies[i].getFillColor() == sf::Color::Red)
							points += 10;
						else if (enemies[i].getFillColor() == sf::Color::Yellow)
							points += 5;
						else if (enemies[i].getFillColor() == sf::Color::Blue)
							points += 3;
						else points += 1;
						std::cout << "Points: " << points << "\n";

						enemies.erase(enemies.begin() + i);
						deleted = true;
					}
				}
			}
		}
		else mouseHeld = false;
	}

	void renderText(sf::RenderTarget& target) {
		target.draw(uiText);
	}

	void renderEnemies(sf::RenderTarget& target) {
		for (auto& each : enemies) {
			target.draw(each);
		}
	}

public:
	Game() {
		initializeVariables();
		initEnemies();
		initText();
		initWindow();
	}

	~Game() {
		if (window) delete window;
	}

	//Accessors
	const bool running() const {
		return window->isOpen() && !endGame;
	}

	//Functions

	void update() {
		pollEvent();
		if (!endGame) {
			updateMousePositions();
			updateText();
			updateEnemies();
		}

		if (health <= 0) endGame = true;
	}

	void render() {
		window->clear();

		//Draw game object
		renderEnemies(*window);

		renderText(*window);
		window->display();
	}

};