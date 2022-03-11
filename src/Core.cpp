#include "Core.hpp"

Core::Core(const std::string &title, unsigned int framerate)
{
    srand(time(NULL));
    _window.create(sf::VideoMode(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height), title);
    _window.setFramerateLimit(framerate);
    generateBorders();
    for (int i = 0; i < 300; i++) {
        float x = WALLOFFSET + rand() % static_cast<int>(_window.getSize().x - WALLOFFSET * 2);
        float y = WALLOFFSET + rand() % static_cast<int>(_window.getSize().y - WALLOFFSET * 2);
        _boids.push_back(new Boid(sf::Vector2f{x, y}, 7, 5, 5));
    }
    _placeHolder = nullptr;
    _leftMouseClick = false;
}

Core::~Core()
{
    for (auto it : _boids)
        delete it;
    for (auto it : _obstacles)
        delete it;
}

void Core::generateBorders()
{
    _obstacles.push_back(new sf::FloatRect(WALLOFFSET, WALLOFFSET, static_cast<float>(_window.getSize().x - WALLOFFSET * 2), 1));
    _obstacles.push_back(new sf::FloatRect(WALLOFFSET, static_cast<float>(_window.getSize().y - WALLOFFSET), static_cast<float>(_window.getSize().x - WALLOFFSET * 2), 1));
    _obstacles.push_back(new sf::FloatRect(WALLOFFSET, WALLOFFSET, 1, static_cast<float>(_window.getSize().y - WALLOFFSET * 2)));
    _obstacles.push_back(new sf::FloatRect(static_cast<float>(_window.getSize().x - WALLOFFSET), WALLOFFSET, 1, static_cast<float>(_window.getSize().y - WALLOFFSET * 2)));
}

void Core::createObstacleOnClick()
{

}

void Core::events()
{
    while (_window.pollEvent(_event)) {
        if (_event.type == sf::Event::Closed)
            _window.close();
        if (_event.type == sf::Event::KeyPressed) {
            if (_event.key.code == sf::Keyboard::BackSpace) {
                if (_obstacles.size() > 0) {
                    delete _obstacles.back();
                    _obstacles.pop_back();
                }
            }
        }
        if (_event.type == sf::Event::MouseButtonPressed) {
            if (_event.mouseButton.button == sf::Mouse::Left) {
                _leftMouseClick = true;
                _placeHolder = new sf::FloatRect(static_cast<float>(sf::Mouse::getPosition(_window).x), static_cast<float>(sf::Mouse::getPosition(_window).y), 0, 0);
            }
        }
        if (_event.type == sf::Event::MouseButtonReleased) {
            if (_event.mouseButton.button == sf::Mouse::Left) {
                _leftMouseClick = false;
                _obstacles.push_back(_placeHolder);
                _placeHolder = nullptr;
            }
        }
    }
}

void Core::drawObstacles()
{
    sf::RectangleShape rectangle;
    for (auto it : _obstacles) {
        rectangle.setPosition(sf::Vector2f{it->left, it->top});
        rectangle.setSize(sf::Vector2f{it->width, it->height});
        _window.draw(rectangle);
    }
    if (_placeHolder) {
        rectangle.setPosition(sf::Vector2f{_placeHolder->left, _placeHolder->top});
        rectangle.setSize(sf::Vector2f{_placeHolder->width, _placeHolder->height});
        _window.draw(rectangle);
    }
}

void Core::display()
{
    _window.clear();
    for (auto it : _boids)
        it->draw(_window);
    drawObstacles();
    _window.display();
}

void Core::update()
{
    for (auto it : _boids)
        it->update(_window, _boids, _obstacles);
    if (_leftMouseClick) {
        _placeHolder->width = static_cast<float>(sf::Mouse::getPosition(_window).x) - _placeHolder->left;
        _placeHolder->height = static_cast<float>(sf::Mouse::getPosition(_window).y) - _placeHolder->top;
        if (std::abs(_placeHolder->width) > std::abs(_placeHolder->height)) _placeHolder->height = 1.0;
        else _placeHolder->width = 1.0;
    }
}

void Core::run()
{
    while (_window.isOpen()) {
        events();
        update();
        display();
    }
}