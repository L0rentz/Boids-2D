#include "Boid.hpp"

static int id = 0;

Boid::Boid(const sf::Vector2f position, const int size)
{
    _id = id++;
    _center.position = position;
    _boid = sf::VertexArray(sf::Triangles, 3);
    _boid[0].position = {_center.position.x, _center.position.y - size * 2};
    _boid[1].position = {_center.position.x - size, _center.position.y + size * 2};
    _boid[2].position = {_center.position.x + size, _center.position.y + size * 2};
    _boid[0].color = sf::Color::Red;
    _boid[1].color = sf::Color::Blue;
    _boid[2].color = sf::Color::Green;
}

float Boid::magnitudeVector2f(const sf::Vector2f a, const sf::Vector2f b)
{
    return std::sqrt(std::pow(b.x - a.x, 2) + std::pow(b.y - a.y, 2));
}

float Boid::dotProductVector2f(const sf::Vector2f a, const sf::Vector2f b)
{
    return a.x * b.x + a.y * b.y;
}

float Boid::angleBetweenVector2f(const sf::Vector2f a, const sf::Vector2f b)
{
    return atan2(a.y - b.y, a.x - b.x);
}

sf::Vector2f Boid::rotatePointAroundCenter(sf::Vector2f point, const sf::Vector2f center, const float angleRad)
{
    float sinVal = sin(angleRad);
    float cosVal = cos(angleRad);

    point.x -= center.x;
    point.y -= center.y;

    float xnew = point.x * cosVal - point.y * sinVal;
    float ynew = point.x * sinVal + point.y * cosVal;

    point.x = xnew + center.x;
    point.y = ynew + center.y;

    return point;
}

void Boid::posDebug(const sf::Vector2f position, sf::RenderWindow &window)
{
    sf::CircleShape dot;
    float radius = 2.0;
    dot.setPosition({position.x - radius, position.y - radius});
    dot.setFillColor(sf::Color::White);
    dot.setRadius(radius);
    window.draw(dot);
}

Boid::~Boid()
{
}

void Boid::update(const sf::RenderWindow &window, const sf::Vector2f mousePosition)
{
    sf::Vector2f mappedCenter = static_cast<sf::Vector2f>(window.mapCoordsToPixel(_center.position, window.getView()));
    sf::Vector2f mappedBoid = static_cast<sf::Vector2f>(window.mapCoordsToPixel(_boid[0].position, window.getView()));
    float angleMouse = angleBetweenVector2f(mappedCenter, mousePosition);
    float angleBoid = angleBetweenVector2f(mappedCenter, mappedBoid);
    float angleDif = angleMouse - angleBoid;
    if (std::abs(angleDif) > 0.02) {
        _boid[0].position = rotatePointAroundCenter(_boid[0].position, _center.position, angleDif);
        _boid[1].position = rotatePointAroundCenter(_boid[1].position, _center.position, angleDif);
        _boid[2].position = rotatePointAroundCenter(_boid[2].position, _center.position, angleDif);
    }
}

void Boid::draw(sf::RenderWindow &window)
{
    window.draw(_boid);
}