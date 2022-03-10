#include "Core.hpp"

Core::Core(const std::string &title, unsigned int framerate)
{
    _window.create(sf::VideoMode(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height), title);
    _window.setFramerateLimit(framerate);
    for (int i = 0; i < 80000; i++) {
        float x = rand() % _window.getSize().x;
        float y = rand() % _window.getSize().y;
        _boids.push_back(new Boid(sf::Vector2f{x, y}, 5));
    }
}

Core::~Core()
{
    for (auto it : _boids)
        delete it;
}

void Core::events()
{
    while (_window.pollEvent(_event)) {
        if (_event.type == sf::Event::Closed)
            _window.close();
    }
}

void Core::display()
{
    _window.clear();
    for (auto it : _boids)
        it->draw(_window);
    _window.display();
}

void Core::update()
{
    sf::Vector2f mousePosition = static_cast<sf::Vector2f>(sf::Mouse::getPosition(_window));
    for (auto it : _boids)
        it->update(_window, mousePosition);
}

void Core::run()
{
    while (_window.isOpen()) {
        events();
        update();
        display();
    }
}