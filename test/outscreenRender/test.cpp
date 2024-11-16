#include "seeker/logger.h"
#include "seeker/loggerApi.h"
#include "component/module.h"

#include <SFML/Graphics.hpp>

int main() {
	sf::RenderWindow* wnd = new sf::RenderWindow(sf::VideoMode(640, 480), L"window");
	sf::Event event;
	sf::Vector2u wndSize = wnd->getSize();
	sf::RectangleShape shape;
	shape.setSize(sf::Vector2f(100, 100));
	shape.setPosition(sf::Vector2f(540, 0));
	shape.setFillColor(sf::Color::Green);

	//alllink::VariableStateRectangleModule shape;
	//shape.set(50, 30, wndSize.x - 100, 0);
	//shape.setShapeSize(10, 10);
	//shape.setShapeColor(sf::Color(0, 0, 0, 0), sf::Color::Black, 1);
	//shape.setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));

	float wr = 0.f;
	float hr = 0.f;

	while (wnd->isOpen()) {
		while (wnd->pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				wnd->close();
				break;
			}
			else if (event.type == sf::Event::Resized) {
				I_LOG("current wnd size is {}:{}, event size is {}:{}", wndSize.x, wndSize.y, event.size.width, event.size.height);
				wr = (float)wndSize.x / event.size.width;
				hr = (float)wndSize.y / event.size.height;
				I_LOG("wr:{} hr:{}", wr, hr);
				wndSize.x = event.size.width;
				wndSize.y = event.size.height;
				int offsetX = event.size.width - wndSize.x;
				int offsetY = event.size.height - wndSize.y;
				sf::Vector2f shapePos = shape.getPosition();
				shape.scale(wr, hr); 
				shapePos.x = wndSize.x - shape.getSize().x;
				shapePos.y = wndSize.y - shape.getSize().y;
				I_LOG("[pos] shape pos is {}:{}", shape.getPosition().x, shape.getPosition().y);
				shape.setPosition(shapePos);
				I_LOG("[pos] shape pos is {}:{}", shape.getPosition().x, shape.getPosition().y);
			}
		}

		wnd->clear(sf::Color(225, 225, 225));
		wnd->draw(shape);
		wnd->display();
	}
	delete wnd;
	return 0;
}