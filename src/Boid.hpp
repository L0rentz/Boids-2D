#ifndef BOID_HPP_
#define BOID_HPP_

#include "Utils.hpp"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "glad/glad.h"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>

#define WALLOFFSET -1.0

class Boid {
    public:
        Boid(const glm::vec2 position, const float gridSize, const int scale = 20, const int speed = 10, const int rotationSpeed = 5);
        ~Boid();

        void update(const sf::Vector2u windowSize, const std::map<int, std::vector<Boid *>> &hashtable, const std::vector<sf::FloatRect *> &obstacles);
        int getGridID() const;
        const glm::vec2 &getWorldPosition() const;
        const glm::vec2 &getScale() const;
        float getAngleDeg() const;

        static void prepareDrawingBuffers(const unsigned int VAO, const unsigned int VBO, const unsigned int instanceVBO);
        static void clearDrawingBuffers(const unsigned int VAO);

    protected:
        static const unsigned int vSize;
        static float vertices[];
        static int nextID;

    private:
        int _id;

        Utils utils;
        glm::vec2 _debugPos;

        glm::mat4 _model;
        glm::vec2 _front;
        glm::vec2 _center;
        glm::vec2 _scale;

        // Algo
        std::vector<std::pair<Boid *, std::pair<float, unsigned int>>> _inRange;
        int _diameter;
        int _radius;
        int _rotationSpeed;
        int _speed;
        unsigned int _fov;
        float _cohesionRange;
        float _alignmentRange;
        float _separationRange;
        float _maxRange;
        int _fleet;
        float _angleDeg;

        // Spatial hashing
        int _gridCell;
        int _cellSize;
        int _width;
        int _buckets;

        bool wallAvoidance(sf::Vector2u windowSize, const std::vector<sf::FloatRect *> &obstacles);
        void rotateBoidTowardPoint(const glm::vec2 point, const glm::vec2 center, const glm::vec2 front);
        void updatePos(sf::Vector2u windowSize);
        void checkBorder(sf::Vector2u windowSize);
        void move(const glm::vec2 translation);
        bool separation();
        bool cohesion();
        bool alignment();
        void findInRange(const std::map<int, std::vector<Boid *>> &hashtable);
        bool processRotation(const glm::vec2 point);
        void updateGridID();
        void setVerticeModel(float x, float y, unsigned int i);
        void setPosition(const glm::vec2 position);
};

#endif /* !BOID_HPP_ */
