#ifndef BOID_HPP_
#define BOID_HPP_

#include "Utils.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>

#define WALLOFFSET -1.0

class Boid {
    public:
        Boid(const sf::Vector2f position, const unsigned int size = 20, const int speed = 10, const unsigned int rotationSpeed = 5);
        ~Boid();

        void draw(sf::RenderWindow &window);
        void update(const sf::RenderWindow &window, const std::vector<Boid *> &boids, const std::vector<sf::FloatRect *> &obstacles);

    protected:
    private:
        int _id;
        Utils utils;

        sf::Vector2f _debugPos;

        sf::Vertex _center;
        sf::VertexArray _boid;
        std::vector<std::pair<Boid *, std::pair<float, unsigned int>>> _inRange;
        unsigned int _size;
        unsigned int _rotationSpeed;
        int _speed;
        unsigned int _fov;
        float _cohesionRange;
        float _alignmentRange;
        float _separationRange;
        float _maxRange;
        sf::Color _color;

        void setColor(const sf::Color color);
        bool wallAvoidance(const sf::RenderWindow &window, const std::vector<sf::FloatRect *> &obstacles, const sf::Vector2f mappedCenter, const sf::Vector2f mappedFront);
        void rotateBoidTowardPoint(const sf::Vector2f point, const sf::Vector2f mappedCenter, const sf::Vector2f mappedFront);
        void moveBoid(const sf::Vector2f windowSize);
        void checkBorder(const sf::Vector2f windowSize);
        void translateBoid(const sf::Vector2f translation);
        bool separation(const sf::RenderWindow &window, const sf::Vector2f mappedCenter, const sf::Vector2f mappedFront);
        bool cohesion(const sf::RenderWindow &window, const sf::Vector2f mappedCenter, const sf::Vector2f mappedFront);
        bool alignment(const sf::RenderWindow &window, const sf::Vector2f mappedCenter, const sf::Vector2f mappedFront);
        void findInRange(const std::vector<Boid *> &boids, const sf::RenderWindow &window, const sf::Vector2f mappedCenter, const sf::Vector2f mappedFront);
        bool processRotation(const sf::Vector2f point, const sf::Vector2f mappedCenter, const sf::Vector2f mappedFront);
};

#endif /* !BOID_HPP_ */
