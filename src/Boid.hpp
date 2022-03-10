#ifndef BOID_HPP_
#define BOID_HPP_

#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

class Boid {
    public:
        Boid(const sf::Vector2f position, const int size);
        ~Boid();

        void draw(sf::RenderWindow &window);
        void update(const sf::RenderWindow &window, const sf::Vector2f mousePosition);

    protected:
    private:
        sf::Vertex _center;
        sf::VertexArray _boid;
        int _id;

        float angleBetweenVector2f(const sf::Vector2f a, const sf::Vector2f b);
        float magnitudeVector2f(const sf::Vector2f a, const sf::Vector2f b);
        float dotProductVector2f(const sf::Vector2f a, const sf::Vector2f b);
        sf::Vector2f rotatePointAroundCenter(sf::Vector2f point, const sf::Vector2f center, const float angleRad);
        void posDebug(const sf::Vector2f position, sf::RenderWindow &window);
};

#endif /* !BOID_HPP_ */
