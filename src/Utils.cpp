#include "Utils.hpp"

Utils::Utils()
{
}

Utils::~Utils()
{
}

unsigned int Utils::mappedDegrees(float radians)
{
    float degrees = radians * (180.0 / M_PI);
    return static_cast<int>(degrees + 360) % 360;
}

unsigned int Utils::mappedDegreesAngleDif(const sf::Vector2f a, const sf::Vector2f tail, const sf::Vector2f b)
{
    float anglePoint = angleBetweenVector2f(tail, a);
    float angleBoid = angleBetweenVector2f(tail, b);
    float angleDif = anglePoint - angleBoid;
    float degreesDif = angleDif * (180.0 / M_PI);
    unsigned int mappedDegrees = static_cast<int>(degreesDif + 360) % 360;

    return mappedDegrees;
}

float Utils::magnitudeVector2f(const sf::Vector2f a, const sf::Vector2f b)
{
    return std::sqrt(std::pow(b.x - a.x, 2) + std::pow(b.y - a.y, 2));
}

float Utils::dotProductVector2f(const sf::Vector2f a, const sf::Vector2f b)
{
    return a.x * b.x + a.y * b.y;
}

float Utils::angleBetweenVector2f(const sf::Vector2f a, const sf::Vector2f b)
{
    return atan2(a.y - b.y, a.x - b.x);
}

sf::Vector2f Utils::rotatePointAroundCenter(sf::Vector2f point, const sf::Vector2f center, const float degrees)
{
    float angleRad = degrees * M_PI / 180;

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

sf::Vector2f Utils::normalizeVector2f(sf::Vector2f a, sf::Vector2f b)
{
    float magnitude = magnitudeVector2f(a, b);
    sf::Vector2f direction = sf::Vector2f{a.x - b.x, a.y - b.y};
    sf::Vector2f normalize = direction;

    normalize.x /= magnitude;
    normalize.y /= magnitude;

    return (normalize);
}

void Utils::posDebug(const sf::Vector2f position, sf::RenderWindow &window)
{
    sf::CircleShape dot;
    float radius = 5.0;
    dot.setPosition({position.x - radius, position.y - radius});
    dot.setFillColor(sf::Color::White);
    dot.setRadius(radius);
    window.draw(dot);
}

bool Utils::segmentIntersectsRectangle(const sf::FloatRect &rect, const sf::Vector2f &a_p1, const sf::Vector2f &a_p2)
{
    // Find min and max X for the segment
    auto minX = std::min(a_p1.x, a_p2.x);
    auto maxX = std::max(a_p1.x, a_p2.x);

    // Find the intersection of the segment's and rectangle's x-projections
    if (maxX > rect.left + rect.width) {
        maxX = rect.left + rect.width;
    }

    if (minX < rect.left) {
        minX = rect.left;
    }

    // If Y-projections do not intersect then there's no intersection
    if (minX > maxX) { return false; }

    // Find corresponding min and max Y for min and max X we found before
    auto minY = a_p1.y;
    auto maxY = a_p2.y;

    auto dx = a_p2.x - a_p1.x;
    if (std::abs(dx) > 0.0000001f) {
        auto k = (a_p2.y - a_p1.y) / dx;
        auto b = a_p1.y - k * a_p1.x;
        minY = k * minX + b;
        maxY = k * maxX + b;
    }

    if (minY > maxY) {
        std::swap(minY, maxY);
    }

    // Find the intersection of the segment's and rectangle's y-projections
    if (maxY > rect.top + rect.height) {
        maxY = rect.top + rect.height;
    }

    if (minY < rect.top) {
        minY = rect.top;
    }

    // If Y-projections do not intersect then there's no intersection
    if (minY > maxY) { return false; }
    return true;
}