#ifndef BOID_HPP_
#define BOID_HPP_

#include "Utils.hpp"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>

#define WALLOFFSET -1.0

#define BOIDS_COUNT 100
#define BUCKETS_COUNT 4 // Change in GLSL compute shader too !

class Boid {
    public:
        Boid();
        Boid(const glm::vec2 position, const float screenWidth, const int scale = 20, const int speed = 10, const int rotationSpeed = 5);
        ~Boid();

        void update(const sf::Vector2u windowSize, const std::map<int, std::vector<Boid *>> &hashtable, const std::vector<sf::FloatRect *> &obstacles);
        int getGridID() const;
        const glm::vec2 &getWorldPosition() const;
        const glm::vec2 &getScale() const;
        double getAngleDeg() const;

        static void prepareDrawingBuffers(unsigned int VAO, unsigned int VBO, unsigned int instanceVBO, float *worldPosScaleAngleDeg);
        static void clearDrawingBuffers(const unsigned int VAO);
        static void updateHashtable(float *hashtable, unsigned int tableSize, float *worldPosScaleAngleDeg, unsigned int worldPosScaleAngleDegOffset, float *bufferSelector);

        glm::vec2 center;
        glm::vec2 scale;
        double angleDeg;

    protected:
    private:
        static const unsigned int _vSize;
        static float _vertices[];
        static int _nextID;
        static float _screenWidth;

        // Spatial hashing
        static int _cellWidth;
        static int _gridWidth;

        int _id;

        Utils utils;
        glm::vec2 _debugPos;

        glm::mat4 _model;
        glm::vec2 _front;

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
        // double _angleDeg;

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
