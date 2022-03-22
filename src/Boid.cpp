#include "Boid.hpp"
#include "glad/glad.h"


// --------------- Static --------------- //


int Boid::_nextID = 0;

const unsigned int Boid::_vSize = 15;

float Boid::_vertices[_vSize] = {
    // Positions   // Colors
     0.0f,  1.0f,  0.5f, 0.8f, 1.0f,
     1.0f, -1.0f,  0.5f, 0.8f, 1.0f,
    -1.0f, -1.0f,  0.5f, 0.8f, 1.0f
};

float Boid::_screenWidth;
int Boid::_cellWidth;
int Boid::_gridWidth;

void Boid::prepareDrawingBuffers(unsigned int VAO, unsigned int VBO, unsigned int instanceVBO, float *worldPosScaleAngleDeg)
{
    std::size_t floatSize = sizeof(float);
    std::size_t vec4Size = sizeof(glm::vec4);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(5 * floatSize), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(5 * floatSize), (void *)(2 * floatSize));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, (vec4Size + floatSize * 2) * BOIDS_COUNT, worldPosScaleAngleDeg, GL_STREAM_DRAW);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(vec4Size + floatSize * 2), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(vec4Size + floatSize * 2), (void*)(4 * floatSize));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
}

void Boid::clearDrawingBuffers(unsigned int VAO)
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexArrayAttrib(VAO, 0);
    glDisableVertexArrayAttrib(VAO, 1);
    glDisableVertexArrayAttrib(VAO, 2);
}

void Boid::updateHashtable(float *hashtable, unsigned int tableSize, float *worldPosScaleAngleDeg, unsigned int worldPosScaleAngleDegOffset, float *bufferSelector)
{
    std::memset(hashtable, 0, tableSize * sizeof(float));
    for (unsigned int i = 0; i < BOIDS_COUNT * worldPosScaleAngleDegOffset; i += worldPosScaleAngleDegOffset) {
        int hashKey = static_cast<int>(std::floor(worldPosScaleAngleDeg[i] / _cellWidth) + std::floor(worldPosScaleAngleDeg[i + 1] / _cellWidth) * _gridWidth);
        if (hashKey < 0 || hashKey >= BUCKETS_COUNT) {
            hashtable[static_cast<int>(worldPosScaleAngleDeg[i + worldPosScaleAngleDegOffset - 1]) * 2]++;
            continue;
        }
        worldPosScaleAngleDeg[i + worldPosScaleAngleDegOffset - 1] = static_cast<float>(hashKey);
        hashtable[hashKey * 2]++;
    }
    for (unsigned int i = 0, totalBoids = 0; i < tableSize; i += 2) {
        hashtable[i + 1] = static_cast<float>(totalBoids);
        totalBoids += static_cast<unsigned int>(hashtable[i]);
    }
    if (*bufferSelector == 1.0f) *bufferSelector = 2.0f;
    else *bufferSelector = 1.0f;
}


// -------------------------------------- //


Boid::Boid()
{
}

Boid::Boid(const glm::vec2 position, const float screenWidth, const int size, const int speed, const int rotationSpeed)
{
    _id = _nextID++;
    _radius = size;
    _diameter = _radius * 2;
    scale = glm::vec2{_diameter, _diameter};

    center = glm::vec3{position.x, position.y, 0.0f};
    glm::vec2 front = glm::vec2{center.x, center.y - _diameter};
    if (_id == 0) {
        setVerticeModel(front.x, front.y, 0);
        setVerticeModel(center.x - _radius, center.y + _diameter, 5);
        setVerticeModel(center.x + _radius, center.y + _diameter, 10);
    }

    angleDeg = rand() % 360;

    _screenWidth = screenWidth;
    _gridWidth = static_cast<int>(glm::sqrt(BUCKETS_COUNT));
    _cellWidth = static_cast<int>(_screenWidth / _gridWidth);
}

Boid::~Boid()
{
}

void Boid::setVerticeModel(float x, float y, unsigned int i)
{
    if (i >= _vSize || i % 5 != 0) return;
    x = center.x - x;
    y = center.y - y;
    glm::vec2 normalized = glm::normalize(glm::vec2{x, y});
    _vertices[i] = normalized.x;
    _vertices[i + 1] = normalized.y;
}

const glm::vec2 &Boid::getWorldPosition() const
{
    return center;
}

const glm::vec2 &Boid::getScale() const
{
    return scale;
}

double Boid::getAngleDeg() const
{
    return angleDeg;
}

// bool Boid::wallAvoidance(sf::Vector2u windowSize, const std::vector<sf::FloatRect *> &obstacles)
// {
//     glm::vec2 normalizedDirection = {0, 0};

//     // Check if outside
//     sf::FloatRect wallRect = {WALLOFFSET, WALLOFFSET, static_cast<float>(windowSize.x - WALLOFFSET * 2), static_cast<float>(windowSize.y - WALLOFFSET * 2)};
//     if (!wallRect.contains(center.x, center.y)) {
//         glm::vec2 mapCenter = glm::vec2{static_cast<float>(windowSize.x / 2), static_cast<float>(windowSize.y / 2)};
//         return processRotation(mapCenter);
//     }

//     // Check if wall
//     normalizedDirection = utils.normalizeVector2f(_front, center);
//     glm::vec2 frontRaycast = glm::vec2{_front.x + normalizedDirection.x * 200, _front.y + normalizedDirection.y * 200};
//     // _debugPos = frontRaycast;
//     int isColliding = 0;
//     for (auto it : obstacles)
//         isColliding += utils.segmentIntersectsRectangle(*it, center, frontRaycast);
//     if (isColliding == 0) return false;
//     glm::vec2 leftRaycast = frontRaycast;
//     glm::vec2 rightRaycast = frontRaycast;
//     for (int raycastAngle = 10; isColliding > 0 && raycastAngle <= 180; raycastAngle += 10) {
//         isColliding = 0;
//         leftRaycast = utils.rotatePointAroundCenter(leftRaycast, center, raycastAngle);
//         for (auto it : obstacles)
//             isColliding += utils.segmentIntersectsRectangle(*it, center, leftRaycast);
//         if (isColliding == 0) {
//             _front = utils.rotatePointAroundCenter(_front, center, _rotationSpeed);
//             angleDeg += _rotationSpeed;
//             return true;
//         }

//         isColliding = 0;
//         rightRaycast = utils.rotatePointAroundCenter(rightRaycast, center, -raycastAngle);
//         for (auto it : obstacles)
//             isColliding += utils.segmentIntersectsRectangle(*it, center, rightRaycast);
//         if (isColliding == 0) {
//             _front = utils.rotatePointAroundCenter(_front, center, -_rotationSpeed);
//             angleDeg -= _rotationSpeed;
//             return true;
//         }
//     }
//     return true;
// }