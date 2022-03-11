#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <SFML/Graphics.hpp>
#include <cmath>

class Utils {
    public:
        Utils();
        ~Utils();

        float angleBetweenVector2f(const sf::Vector2f a, const sf::Vector2f b);
        float magnitudeVector2f(const sf::Vector2f a, const sf::Vector2f b);
        float dotProductVector2f(const sf::Vector2f a, const sf::Vector2f b);
        sf::Vector2f rotatePointAroundCenter(sf::Vector2f point, const sf::Vector2f center, const float degrees);
        sf::Vector2f normalizeVector2f(sf::Vector2f a, sf::Vector2f b);
        void posDebug(const sf::Vector2f position, sf::RenderWindow &window);
        unsigned int mappedDegreesAngleDif(const sf::Vector2f a, const sf::Vector2f tail, const sf::Vector2f b);
        unsigned int mappedDegrees(float radians);
        bool segmentIntersectsRectangle(const sf::FloatRect &rect, const sf::Vector2f &a_p1, const sf::Vector2f &a_p2);

    protected:
    private:
};

#endif /* !UTILS_HPP_ */
