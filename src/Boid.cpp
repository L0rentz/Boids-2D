#include "Boid.hpp"

int Boid::nextID = 0;

const unsigned int Boid::vSize = 15;

float Boid::vertices[vSize] = {
    // Positions   // Colors
     0.0f,  1.0f,  1.0f, 1.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f,  1.0f, 1.0f, 1.0f
};

Boid::Boid(const glm::vec2 position, const float gridSize, const unsigned int size, const int speed, const unsigned int rotationSpeed)
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
    _size = size;

    _pos = glm::vec3{position.x, position.y, 0.0f};
    _front = glm::vec2{_pos.x, _pos.y - _size * 2};
    if (_id == 0) {
        setVerticeModel(_pos.x, _pos.y - _size * 2, 0);
        setVerticeModel(_pos.x - _size, _pos.y + _size * 2, 5);
        setVerticeModel(_pos.x + _size, _pos.y + _size * 2, 10);
    }
    _model = glm::mat4(1.0f);

    _angleDeg = rand() % 360;
    _front = utils.rotatePointAroundCenter(_front, _pos, _angleDeg);

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
    x = _pos.x - x;
    y = _pos.y - y;
    glm::vec2 normalized = glm::normalize((glm::vec2){x, y});
    vertices[i] = normalized.x;
    vertices[i + 1] = normalized.y;
}

glm::mat4 Boid::getModelMatrix(const sf::RenderWindow &window) const
{
    sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i{(int)_pos.x, (int)_pos.y});
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(worldPos.x, worldPos.y, 0.0f));

    model = glm::translate(model, glm::vec3(0.5f * _size, 0.5f * _size, 0.0f));
    model = glm::rotate(model, glm::radians(_angleDeg), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-0.5f * _size, -0.5f * _size, 0.0f));

    model = glm::scale(model, glm::vec3((float)_size * 2, (float)_size * 2, 1.0f));
    return model;
}

void Boid::prepareDrawingBuffers(unsigned int VAO, unsigned int VBO, unsigned int instanceVBO)
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Boid::vertices), Boid::vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    std::size_t vec4Size = sizeof(glm::vec4);
    for (int i = 0; i < 4; i++) {
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * i));
        glEnableVertexAttribArray(2 + i);
        glVertexAttribDivisor(2 + i, 1);
    }
}

void Boid::clearDrawingBuffers(unsigned int VAO)
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexArrayAttrib(VAO, 0);
    glDisableVertexArrayAttrib(VAO, 1);
    glDisableVertexArrayAttrib(VAO, 2);
}

void Boid::updateGridID()
{
    _gridCell = std::floor(_pos.x / _cellSize) + std::floor(_pos.y / _cellSize) * _width;
}

int Boid::getGridID() const
{
    return (_gridCell);
}

void Boid::rotateBoidTowardPoint(const glm::vec2 point, const glm::vec2 center, const glm::vec2 front)
{
    unsigned int mappedDegrees = utils.mappedDegreesAngleDif(point, center, front);
    if (mappedDegrees > _rotationSpeed) {
        int rotationSpeed = _rotationSpeed;
        if (mappedDegrees > 180) rotationSpeed *= -1;
        _front = utils.rotatePointAroundCenter(_front, _pos, rotationSpeed);
        _angleDeg += rotationSpeed;
    }
}

void Boid::move(const glm::vec2 translation)
{
    _pos.x += translation.x;
    _pos.y += translation.y;
    _pos = glm::vec3{_pos.x, _pos.y, 0.0f};
    _front.x += translation.x;
    _front.y += translation.y;
}

void Boid::setPosition(const glm::vec2 position)
{
    _front.x -= _pos.x;
    _front.y -= _pos.y;
    _pos.x = position.x;
    _pos.y = position.y;
    _front += _pos.x;
    _front += _pos.y;
}

void Boid::checkBorder(const sf::Vector2u windowSize)
{
    int halfLengthBoid = _size * 2;
    if (_pos.x > windowSize.x + halfLengthBoid) setPosition(glm::vec2{-halfLengthBoid, _pos.y});
    else if (_pos.x < 0 - halfLengthBoid) setPosition(glm::vec2{windowSize.x + halfLengthBoid, _pos.y});
    else if (_pos.y > windowSize.y + halfLengthBoid) setPosition(glm::vec2{_pos.x, -halfLengthBoid});
    else if (_pos.y < 0 - halfLengthBoid) setPosition(glm::vec2{_pos.x, windowSize.y + halfLengthBoid});
}

void Boid::findInRange(const std::map<int, std::vector<Boid *>> &hashtable)
{
    _inRange.clear();
    float magnitude = 0.0;
    unsigned int mappedDegrees = 0;
    auto hashIt = hashtable.find(getGridID());
    for (auto it : hashIt->second) {
        if (it->_id == _id) continue;
        magnitude = utils.magnitudeVector2f(it->_pos, _pos);
        if (magnitude > _maxRange) continue;
        mappedDegrees = utils.mappedDegreesAngleDif(it->_pos, _pos, _front);
        if (mappedDegrees > _fov / 2 && mappedDegrees < 360 - _fov / 2) continue;
        _inRange.push_back(std::make_pair(it, std::make_pair(magnitude, mappedDegrees)));
    }
}

bool Boid::processRotation(const glm::vec2 point)
{
    glm::vec2 normalizedDirection = utils.normalizeVector2f(_pos, point);
    glm::vec2 normalizedBoidDirection = utils.normalizeVector2f(_pos, _front);
    float angleDirection = std::atan2(normalizedDirection.x, normalizedDirection.y);
    float angleBoid = std::atan2(normalizedBoidDirection.x, normalizedBoidDirection.y);
    unsigned int mappedDirectionDegrees = utils.mappedDegrees(angleDirection);
    unsigned int mappedBoidDegrees = utils.mappedDegrees(angleBoid);
    int degreesDif = mappedDirectionDegrees - mappedBoidDegrees;
    if (std::abs(degreesDif) <= _rotationSpeed) return false;
    int rotationSpeed = _rotationSpeed;
    rotationSpeed *= -1;
    if (mappedDirectionDegrees < mappedBoidDegrees && mappedBoidDegrees - mappedDirectionDegrees <= 180) rotationSpeed *= -1;
    if (mappedDirectionDegrees > mappedBoidDegrees && mappedDirectionDegrees- mappedBoidDegrees > 180) rotationSpeed *= -1;
    _front = utils.rotatePointAroundCenter(_front, _pos, rotationSpeed);
    _angleDeg += rotationSpeed;
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
            total.x += it.first->_pos.x;
            total.y += it.first->_pos.y;
        }
        if (count == 0) return false;
        center.x = (total.x / count - _pos.x) * -1 + _pos.x;
        center.y = (total.y / count - _pos.y) * -1 + _pos.y;
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
            total.x += it.first->_pos.x;
            total.y += it.first->_pos.y;
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
            itNormalizedVelocity = utils.normalizeVector2f(it.first->_pos, it.first->_front);
            total.x += itNormalizedVelocity.x;
            total.y += itNormalizedVelocity.y;
        }
        if (count == 0) return false;
        center.x = (total.x / count) * -1 * _speed + _pos.x;
        center.y = (total.y / count) * -1 * _speed + _pos.y;
    } else return false;
    //_debugPos = center;
    return processRotation(center);
}

bool Boid::wallAvoidance(sf::Vector2u windowSize, const std::vector<sf::FloatRect *> &obstacles)
{
    glm::vec2 normalizedDirection = {0, 0};

    // Check if outside
    sf::FloatRect wallRect = {WALLOFFSET, WALLOFFSET, static_cast<float>(windowSize.x - WALLOFFSET * 2), static_cast<float>(windowSize.y - WALLOFFSET * 2)};
    if (!wallRect.contains(_pos.x, _pos.y)) {
        glm::vec2 mapCenter = glm::vec2{static_cast<float>(windowSize.x / 2), static_cast<float>(windowSize.y / 2)};
        return processRotation(mapCenter);
    }

    // Check if wall
    normalizedDirection = utils.normalizeVector2f(_front, _pos);
    glm::vec2 frontRaycast = glm::vec2{_front.x + normalizedDirection.x * 200, _front.y + normalizedDirection.y * 200};
    // _debugPos = frontRaycast;
    int isColliding = 0;
    for (auto it : obstacles)
        isColliding += utils.segmentIntersectsRectangle(*it, _pos, frontRaycast);
    if (isColliding == 0) return false;
    glm::vec2 leftRaycast = frontRaycast;
    glm::vec2 rightRaycast = frontRaycast;
    for (int raycastAngle = 10; isColliding > 0 && raycastAngle <= 180; raycastAngle += 10) {
        isColliding = 0;
        leftRaycast = utils.rotatePointAroundCenter(leftRaycast, _pos, raycastAngle);
        for (auto it : obstacles)
            isColliding += utils.segmentIntersectsRectangle(*it, _pos, leftRaycast);
        if (isColliding == 0) {
            _front = utils.rotatePointAroundCenter(_front, _pos, _rotationSpeed);
            _angleDeg += _rotationSpeed;
            return true;
        }

        isColliding = 0;
        rightRaycast = utils.rotatePointAroundCenter(rightRaycast, _pos, -raycastAngle);
        for (auto it : obstacles)
            isColliding += utils.segmentIntersectsRectangle(*it, _pos, rightRaycast);
        if (isColliding == 0) {
            int rotationSpeed = -static_cast<float>(_rotationSpeed);
            _front = utils.rotatePointAroundCenter(_front, _pos, rotationSpeed);
            _angleDeg += rotationSpeed;
            return true;
        }
    }
    return true;
}

void Boid::updatePos(const sf::Vector2u windowSize)
{
    glm::vec2 directionNormalized = utils.normalizeVector2f(_front, _pos);
    checkBorder(windowSize);
    move(glm::vec2{(directionNormalized.x * _speed), (directionNormalized.y * _speed)});
    updateGridID();
}

void Boid::update(const sf::Vector2u windowSize, const std::map<int, std::vector<Boid *>> &hashtable, const std::vector<sf::FloatRect *> &obstacles)
{
    bool check = false;
    /*check = wallAvoidance(windowSize, obstacles);
    if (!check && hashtable.size() > 0) {
        findInRange(hashtable);
        check = separation();
        if (!check) check = alignment();
        if (!check) cohesion();
    }*/
    //updatePos(windowSize);
}