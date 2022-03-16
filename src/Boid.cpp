#include "Boid.hpp"


// --------------- Static --------------- //


int Boid::nextID = 0;

const unsigned int Boid::vSize = 15;

float Boid::vertices[vSize] = {
    // Positions   // Colors
     0.0f,  1.0f,  1.0f, 1.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f,  1.0f, 1.0f, 1.0f
};

void Boid::prepareDrawingBuffers(unsigned int VAO, unsigned int VBO, unsigned int instanceVBO)
{
    std::size_t floatSize = sizeof(float);
    std::size_t vec4Size = sizeof(glm::vec4);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Boid::vertices), Boid::vertices, GL_STATIC_DRAW);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * floatSize, (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * floatSize, (void *)(2 * floatSize));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, vec4Size + floatSize, (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, vec4Size + floatSize, (void*)(4 * floatSize));
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


// -------------------------------------- //


Boid::Boid(const glm::vec2 position, const float gridSize, const int size, const int speed, const int rotationSpeed)
{
    _id = nextID++;
    _fleet = _id % 2;
    _cohesionRange = 75.0;
    _alignmentRange = 50.0;
    _separationRange = 15.0;
    _maxRange = _cohesionRange > _alignmentRange ? _cohesionRange > _separationRange ? _cohesionRange : _separationRange : _alignmentRange > _separationRange ? _alignmentRange : _separationRange;
    _fov = 260;
    _rotationSpeed = rotationSpeed;
    _speed = speed;
    _radius = size;
    _diameter = _radius * 2;
    _scale = glm::vec2{_diameter, _diameter};

    _center = glm::vec3{position.x, position.y, 0.0f};
    _front = glm::vec2{_center.x, _center.y - _diameter};
    if (_id == 0) {
        setVerticeModel(_front.x, _front.y, 0);
        setVerticeModel(_center.x - _radius, _center.y + _diameter, 5);
        setVerticeModel(_center.x + _radius, _center.y + _diameter, 10);
    }
    _model = glm::mat4(1.0f);

    _angleDeg = rand() % 360;
    _front = utils.rotatePointAroundCenter(_front, _center, _angleDeg);

    float min = 0.0;
    float max = gridSize;
    _cellSize = gridSize / 10;
    _width = (max - min) / _cellSize;
    _buckets = std::pow(_width, 2);
    updateGridID();
}

Boid::~Boid()
{
}

void Boid::setVerticeModel(float x, float y, unsigned int i)
{
    if (i >= vSize || i % 5 != 0) return;
    x = _center.x - x;
    y = _center.y - y;
    glm::vec2 normalized = glm::normalize((glm::vec2){x, y});
    vertices[i] = normalized.x;
    vertices[i + 1] = normalized.y;
}

const glm::vec2 &Boid::getWorldPosition() const
{
    return _center;
}

const glm::vec2 &Boid::getScale() const
{
    return _scale;
}

float Boid::getAngleDeg() const
{
    return _angleDeg;
}

void Boid::updateGridID()
{
    _gridCell = std::floor(_center.x / _cellSize) + std::floor(_center.y / _cellSize) * _width;
}

int Boid::getGridID() const
{
    return (_gridCell);
}

void Boid::rotateBoidTowardPoint(const glm::vec2 point, const glm::vec2 center, const glm::vec2 front)
{
    int mappedDegrees = utils.mappedDegreesAngleDif(point, center, front);
    if (mappedDegrees > _rotationSpeed) {
        int tmpRotationSpeed = _rotationSpeed;
        if (mappedDegrees > 180) tmpRotationSpeed *= -1;
        _front = utils.rotatePointAroundCenter(_front, _center, tmpRotationSpeed);
        _angleDeg += tmpRotationSpeed;
    }
}

void Boid::move(const glm::vec2 translation)
{
    _center.x += translation.x;
    _center.y += translation.y;
    _center = glm::vec3{_center.x, _center.y, 0.0f};
    _front.x += translation.x;
    _front.y += translation.y;
}

void Boid::setPosition(const glm::vec2 position)
{
    _front.x -= _center.x;
    _front.y -= _center.y;
    _center.x = position.x;
    _center.y = position.y;
    _front += _center.x;
    _front += _center.y;
}

void Boid::checkBorder(const sf::Vector2u windowSize)
{
    if (_center.x > windowSize.x + _diameter) setPosition(glm::vec2{-_diameter, _center.y});
    else if (_center.x < 0 - _diameter) setPosition(glm::vec2{windowSize.x + _diameter, _center.y});
    else if (_center.y > windowSize.y + _diameter) setPosition(glm::vec2{_center.x, -_diameter});
    else if (_center.y < 0 - _diameter) setPosition(glm::vec2{_center.x, windowSize.y + _diameter});
}

void Boid::findInRange(const std::map<int, std::vector<Boid *>> &hashtable)
{
    _inRange.clear();
    float magnitude = 0.0;
    unsigned int mappedDegrees = 0;
    auto hashIt = hashtable.find(getGridID());
    for (auto it : hashIt->second) {
        if (it->_id == _id) continue;
        magnitude = utils.magnitudeVector2f(it->_center, _center);
        if (magnitude > _maxRange) continue;
        mappedDegrees = utils.mappedDegreesAngleDif(it->_center, _center, _front);
        if (mappedDegrees > _fov / 2 && mappedDegrees < 360 - _fov / 2) continue;
        _inRange.push_back(std::make_pair(it, std::make_pair(magnitude, mappedDegrees)));
    }
}

bool Boid::processRotation(const glm::vec2 point)
{
    glm::vec2 normalizedDirection = utils.normalizeVector2f(_center, point);
    glm::vec2 normalizedBoidDirection = utils.normalizeVector2f(_center, _front);
    float angleDirection = std::atan2(normalizedDirection.x, normalizedDirection.y);
    float angleBoid = std::atan2(normalizedBoidDirection.x, normalizedBoidDirection.y);
    unsigned int mappedDirectionDegrees = utils.mappedDegrees(angleDirection);
    unsigned int mappedBoidDegrees = utils.mappedDegrees(angleBoid);
    int degreesDif = mappedDirectionDegrees - mappedBoidDegrees;
    if (std::abs(degreesDif) <= _rotationSpeed) return false;
    int tmpRotationSpeed = _rotationSpeed;
    tmpRotationSpeed *= -1;
    if (mappedDirectionDegrees < mappedBoidDegrees && mappedBoidDegrees - mappedDirectionDegrees <= 180) tmpRotationSpeed *= -1;
    if (mappedDirectionDegrees > mappedBoidDegrees && mappedDirectionDegrees- mappedBoidDegrees > 180) tmpRotationSpeed *= -1;
    _front = utils.rotatePointAroundCenter(_front, _center, tmpRotationSpeed);
    _angleDeg += tmpRotationSpeed;
    return true;
}

bool Boid::separation()
{
    glm::vec2 center = {0, 0};
    if (_inRange.size() > 0) {
        int count = 0;
        glm::vec2 total = {0, 0};
        for (auto it : _inRange) {
            if (it.second.first > _separationRange) continue;
            count++;
            total.x += it.first->_center.x;
            total.y += it.first->_center.y;
        }
        if (count == 0) return false;
        center.x = (total.x / count - _center.x) * -1 + _center.x;
        center.y = (total.y / count - _center.y) * -1 + _center.y;
    } else return false;
    //_debugPos = center;
    return processRotation(center);
}

bool Boid::cohesion()
{
    glm::vec2 center = {0, 0};
    if (_inRange.size() > 0) {
        int count = 0;
        glm::vec2 total = {0, 0};
        for (auto it : _inRange) {
            if (it.second.first > _cohesionRange) continue;
            count++;
            total.x += it.first->_center.x;
            total.y += it.first->_center.y;
        }
        if (count == 0) return false;
        center.x = total.x / count;
        center.y = total.y / count;
    } else return false;
    //_debugPos = center;
    return processRotation(center);
}

bool Boid::alignment()
{
    glm::vec2 center = {0, 0};
    if (_inRange.size() > 0) {
        int count = 0;
        glm::vec2 total = {0, 0};
        glm::vec2 itNormalizedVelocity = {0, 0};
        for (auto it : _inRange) {
            if (it.second.first > _alignmentRange) continue;
            count++;
            itNormalizedVelocity = utils.normalizeVector2f(it.first->_center, it.first->_front);
            total.x += itNormalizedVelocity.x;
            total.y += itNormalizedVelocity.y;
        }
        if (count == 0) return false;
        center.x = (total.x / count) * -1 * _speed + _center.x;
        center.y = (total.y / count) * -1 * _speed + _center.y;
    } else return false;
    //_debugPos = center;
    return processRotation(center);
}

bool Boid::wallAvoidance(sf::Vector2u windowSize, const std::vector<sf::FloatRect *> &obstacles)
{
    glm::vec2 normalizedDirection = {0, 0};

    // Check if outside
    sf::FloatRect wallRect = {WALLOFFSET, WALLOFFSET, static_cast<float>(windowSize.x - WALLOFFSET * 2), static_cast<float>(windowSize.y - WALLOFFSET * 2)};
    if (!wallRect.contains(_center.x, _center.y)) {
        glm::vec2 mapCenter = glm::vec2{static_cast<float>(windowSize.x / 2), static_cast<float>(windowSize.y / 2)};
        return processRotation(mapCenter);
    }

    // Check if wall
    normalizedDirection = utils.normalizeVector2f(_front, _center);
    glm::vec2 frontRaycast = glm::vec2{_front.x + normalizedDirection.x * 200, _front.y + normalizedDirection.y * 200};
    // _debugPos = frontRaycast;
    int isColliding = 0;
    for (auto it : obstacles)
        isColliding += utils.segmentIntersectsRectangle(*it, _center, frontRaycast);
    if (isColliding == 0) return false;
    glm::vec2 leftRaycast = frontRaycast;
    glm::vec2 rightRaycast = frontRaycast;
    for (int raycastAngle = 10; isColliding > 0 && raycastAngle <= 180; raycastAngle += 10) {
        isColliding = 0;
        leftRaycast = utils.rotatePointAroundCenter(leftRaycast, _center, raycastAngle);
        for (auto it : obstacles)
            isColliding += utils.segmentIntersectsRectangle(*it, _center, leftRaycast);
        if (isColliding == 0) {
            _front = utils.rotatePointAroundCenter(_front, _center, _rotationSpeed);
            _angleDeg += _rotationSpeed;
            return true;
        }

        isColliding = 0;
        rightRaycast = utils.rotatePointAroundCenter(rightRaycast, _center, -raycastAngle);
        for (auto it : obstacles)
            isColliding += utils.segmentIntersectsRectangle(*it, _center, rightRaycast);
        if (isColliding == 0) {
            _front = utils.rotatePointAroundCenter(_front, _center, -_rotationSpeed);
            _angleDeg -= _rotationSpeed;
            return true;
        }
    }
    return true;
}

void Boid::updatePos(const sf::Vector2u windowSize)
{
    glm::vec2 directionNormalized = utils.normalizeVector2f(_front, _center);
    // checkBorder(windowSize);
    move(glm::vec2{(directionNormalized.x * _speed), (directionNormalized.y * _speed)});
    updateGridID();
}

void Boid::update(const sf::Vector2u windowSize, const std::map<int, std::vector<Boid *>> &hashtable, const std::vector<sf::FloatRect *> &obstacles)
{
    bool check = false;
    check = wallAvoidance(windowSize, obstacles);
    if (!check && hashtable.size() > 0) {
        findInRange(hashtable);
        check = separation();
        if (!check) check = alignment();
        if (!check) cohesion();
    }
    updatePos(windowSize);
}