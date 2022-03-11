#include "Boid.hpp"

static int id = 0;

Boid::Boid(const sf::Vector2f position, const unsigned int size, const int speed, const unsigned int rotationSpeed)
{
    _id = id++;
    _cohesionRange = 100.0;
    _alignmentRange = 75.0;
    _separationRange = 25.0;
    _maxRange = _cohesionRange > _alignmentRange ? _cohesionRange > _separationRange ? _cohesionRange : _separationRange : _alignmentRange > _separationRange ? _alignmentRange : _separationRange;
    _fov = 260;
    _rotationSpeed = rotationSpeed;
    _speed = speed;
    _size = size;
    _center.position = position;
    _boid = sf::VertexArray(sf::Triangles, 3);
    _boid[0].position = {_center.position.x, _center.position.y - _size * 2};
    _boid[1].position = {_center.position.x - _size, _center.position.y + _size * 2};
    _boid[2].position = {_center.position.x + _size, _center.position.y + _size * 2};
    _color = sf::Color(7, 87, 152, 255);
    if (_id % 2 == 1) _color = sf::Color(121, 158, 196, 255);
    setColor(_color);
    int randRotation = rand() % 360;
    for (int i = 0; i < 3; i++)
       _boid[i].position = utils.rotatePointAroundCenter(_boid[i].position, _center.position, randRotation);
}

Boid::~Boid()
{
}

void Boid::rotateBoidTowardPoint(const sf::Vector2f point, const sf::Vector2f mappedCenter, const sf::Vector2f mappedFront)
{
    unsigned int mappedDegrees = utils.mappedDegreesAngleDif(point, mappedCenter, mappedFront);
    if (mappedDegrees > _rotationSpeed) {
        int rotationSpeed = _rotationSpeed;
        if (mappedDegrees > 180) rotationSpeed *= -1;
        for (int i = 0; i < 3; i++)
            _boid[i].position = utils.rotatePointAroundCenter(_boid[i].position, _center.position, rotationSpeed);
    }
}

void Boid::translateBoid(const sf::Vector2f translation)
{
    _center.position.x += translation.x;
    _center.position.y += translation.y;
    for (int i = 0; i < 3; i++) {
        _boid[i].position.x += translation.x;
        _boid[i].position.y += translation.y;
    }
}

void Boid::checkBorder(const sf::Vector2f windowSize)
{
    int halfLengthBoid = _size * 2;
    if (_center.position.x > windowSize.x + halfLengthBoid) translateBoid(sf::Vector2f{-(windowSize.x + halfLengthBoid * 2), 0});
    if (_center.position.x < 0 - halfLengthBoid) translateBoid(sf::Vector2f{windowSize.x + halfLengthBoid * 2, 0});
    if (_center.position.y > windowSize.y + halfLengthBoid) translateBoid(sf::Vector2f{0, -(windowSize.y + halfLengthBoid * 2)});
    if (_center.position.y < 0 - halfLengthBoid) translateBoid(sf::Vector2f{0, windowSize.y + halfLengthBoid * 2});
}

void Boid::setColor(const sf::Color color)
{
    _boid[0].color = color;
    _boid[1].color = color;
    _boid[2].color = color;
}

void Boid::findInRange(const std::vector<Boid *> &boids, const sf::RenderWindow &window, const sf::Vector2f mappedCenter, const sf::Vector2f mappedFront)
{
    _inRange.clear();
    float magnitude = 0.0;
    unsigned int mappedDegrees = 0;
    for (auto it : boids) {
        if (it->_id == _id) continue;
        sf::Vector2f itMappedCenter = static_cast<sf::Vector2f>(window.mapCoordsToPixel(it->_center.position, window.getView()));
        magnitude = utils.magnitudeVector2f(itMappedCenter, mappedCenter);
        if (magnitude > _maxRange) {
            if (_id == 1) it->setColor(it->_color);
            continue;
        }
        mappedDegrees = utils.mappedDegreesAngleDif(itMappedCenter, mappedCenter, mappedFront);
        if (mappedDegrees > _fov / 2 && mappedDegrees < 360 - _fov / 2) {
            if (_id == 1) it->setColor(it->_color);
            continue;
        }
        if (_id == 1) it->setColor(sf::Color::Yellow);
        _inRange.push_back(std::make_pair(it, std::make_pair(magnitude, mappedDegrees)));
    }
}

bool Boid::processRotation(const sf::Vector2f point, const sf::Vector2f mappedCenter, const sf::Vector2f mappedFront)
{
    sf::Vector2f normalizedDirection = utils.normalizeVector2f(mappedCenter, point);
    sf::Vector2f normalizedBoidDirection = utils.normalizeVector2f(mappedCenter, mappedFront);
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
    for (int i = 0; i < 3; i++)
        _boid[i].position = utils.rotatePointAroundCenter(_boid[i].position, _center.position, rotationSpeed);
    return true;
}

bool Boid::separation(const sf::RenderWindow &window, const sf::Vector2f mappedCenter, const sf::Vector2f mappedFront)
{
    sf::Vector2f center = {0, 0};
    if (_inRange.size() > 0) {
        int count = 0;
        sf::Vector2f total = {0, 0};
        for (auto it : _inRange) {
            if (it.second.first > _separationRange) continue;
            count++;
            sf::Vector2f mappedOtherCenter = static_cast<sf::Vector2f>(window.mapCoordsToPixel(it.first->_center.position, window.getView()));
            total.x += mappedOtherCenter.x;
            total.y += mappedOtherCenter.y;
        }
        if (count == 0) return false;
        center.x = (total.x / count - mappedCenter.x) * -1 + mappedCenter.x;
        center.y = (total.y / count - mappedCenter.y) * -1 + mappedCenter.y;
    } else return false;
    //_debugPos = center;
    return processRotation(center, mappedCenter, mappedFront);
}

bool Boid::cohesion(const sf::RenderWindow &window, const sf::Vector2f mappedCenter, const sf::Vector2f mappedFront)
{
    sf::Vector2f center = {0, 0};
    if (_inRange.size() > 0) {
        int count = 0;
        sf::Vector2f total = {0, 0};
        for (auto it : _inRange) {
            if (it.second.first > _cohesionRange) continue;
            count++;
            sf::Vector2f mappedOtherCenter = static_cast<sf::Vector2f>(window.mapCoordsToPixel(it.first->_center.position, window.getView()));
            total.x += mappedOtherCenter.x;
            total.y += mappedOtherCenter.y;
        }
        if (count == 0) return false;
        center.x = total.x / count;
        center.y = total.y / count;
    } else return false;
    //_debugPos = center;
    return processRotation(center, mappedCenter, mappedFront);
}

bool Boid::alignment(const sf::RenderWindow &window, const sf::Vector2f mappedCenter, const sf::Vector2f mappedFront)
{
    sf::Vector2f center = {0, 0};
    if (_inRange.size() > 0) {
        int count = 0;
        sf::Vector2f total = {0, 0};
        for (auto it : _inRange) {
            if (it.second.first > _alignmentRange) continue;
            count++;
            sf::Vector2f mappedOtherNormalizedVelocity = utils.normalizeVector2f(static_cast<sf::Vector2f>(window.mapCoordsToPixel(it.first->_center.position, window.getView())), static_cast<sf::Vector2f>(window.mapCoordsToPixel(it.first->_boid[0].position, window.getView())));
            total.x += mappedOtherNormalizedVelocity.x;
            total.y += mappedOtherNormalizedVelocity.y;
        }
        if (count == 0) return false;
        center.x = (total.x / count) * -1 * _speed + mappedCenter.x;
        center.y = (total.y / count) * -1 * _speed + mappedCenter.y;
    } else return false;
    //_debugPos = center;
    return processRotation(center, mappedCenter, mappedFront);
}

bool Boid::wallAvoidance(const sf::RenderWindow &window, const std::vector<sf::FloatRect *> &obstacles, const sf::Vector2f mappedCenter, const sf::Vector2f mappedFront)
{
    sf::Vector2f normalizedDirection = {0, 0};

    // Check if outside
    sf::FloatRect wallRect = {WALLOFFSET, WALLOFFSET, static_cast<float>(window.getSize().x - WALLOFFSET * 2), static_cast<float>(window.getSize().y - WALLOFFSET * 2)};
    if (!wallRect.contains(_center.position.x, _center.position.y)) {
        sf::Vector2f mapCenter = sf::Vector2f{static_cast<float>(window.getSize().x / 2), static_cast<float>(window.getSize().y / 2)};
        sf::Vector2f mappedMapCenter = static_cast<sf::Vector2f>(window.mapCoordsToPixel(mapCenter, window.getView()));
        return processRotation(mappedMapCenter, mappedCenter, mappedFront);
    }

    // Check if wall
    normalizedDirection = utils.normalizeVector2f(mappedFront, mappedCenter);
    sf::Vector2f frontRaycast = sf::Vector2f{mappedFront.x + normalizedDirection.x * 200, mappedFront.y + normalizedDirection.y * 200};
    // _debugPos = frontRaycast;
    int isColliding = 0;
    for (auto it : obstacles)
        isColliding += utils.segmentIntersectsRectangle(*it, mappedCenter, frontRaycast);
    if (isColliding == 0) return false;
    sf::Vector2f leftRaycast = frontRaycast;
    sf::Vector2f rightRaycast = frontRaycast;
    for (int raycastAngle = 10; isColliding > 0 && raycastAngle <= 180; raycastAngle += 10) {
        isColliding = 0;
        leftRaycast = utils.rotatePointAroundCenter(leftRaycast, mappedCenter, raycastAngle);
        for (auto it : obstacles)
            isColliding += utils.segmentIntersectsRectangle(*it, mappedCenter, leftRaycast);
        if (isColliding == 0) {
            for (int i = 0; i < 3; i++)
                _boid[i].position = utils.rotatePointAroundCenter(_boid[i].position, _center.position, _rotationSpeed);
            return true;
        }

        isColliding = 0;
        rightRaycast = utils.rotatePointAroundCenter(rightRaycast, mappedCenter, -raycastAngle);
        for (auto it : obstacles)
            isColliding += utils.segmentIntersectsRectangle(*it, mappedCenter, rightRaycast);
        if (isColliding == 0) {
            for (int i = 0; i < 3; i++)
                _boid[i].position = utils.rotatePointAroundCenter(_boid[i].position, _center.position, -static_cast<float>(_rotationSpeed));
            return true;
        }
    }
    return true;
}

void Boid::moveBoid(const sf::Vector2f windowSize)
{
    sf::Vector2f directionNormalized = utils.normalizeVector2f(_boid[0].position, _center.position);
    static_cast<void>(windowSize);
    // checkBorder(windowSize);
    translateBoid(sf::Vector2f{(directionNormalized.x * _speed), (directionNormalized.y * _speed)});
}

void Boid::update(const sf::RenderWindow &window, const std::vector<Boid *> &boids, const std::vector<sf::FloatRect *> &obstacles)
{
    sf::Vector2f mappedCenter = static_cast<sf::Vector2f>(window.mapCoordsToPixel(_center.position, window.getView()));
    sf::Vector2f mappedFront = static_cast<sf::Vector2f>(window.mapCoordsToPixel(_boid[0].position, window.getView()));
    sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.mapPixelToCoords(static_cast<sf::Vector2i>(window.getSize()), window.getView()));
    bool check = false;
    check = wallAvoidance(window, obstacles, mappedCenter, mappedFront);
    if (!check) {
        findInRange(boids, window, mappedCenter, mappedFront);
        check = separation(window, mappedCenter, mappedFront);
        if (!check) check = alignment(window, mappedCenter, mappedFront);
        if (!check) cohesion(window, mappedCenter, mappedFront);
    }
    moveBoid(windowSize);
}

void Boid::draw(sf::RenderWindow &window)
{
    if (_id == 1) setColor(sf::Color::Red);
    window.draw(_boid);
    //if (_id == 1) utils.posDebug(sf::Vector2f{_debugPos.x, _debugPos.y}, window);
}