#version 460 core

#define BUCKETS_COUNT 400
#define GRID_WIDTH sqrt(BUCKETS_COUNT)

#define PI 3.1415926538

#define COHESION 40.0f
#define ALIGNMENT 25.0f
#define SEPARATION 3.0f
#define FOV 260
#define ROTATION_SPEED 4
#define SPEED 2
#define RAYCAST_RANGE 50

layout (local_size_x = 32) in;

struct s_hash {
    float size;
    float startIdx;
};

struct s_boid {
    float centerX;
    float centerY;
    float scaleX;
    float scaleY;
    float angleDeg;
    float hashKey;
};

layout(std430, binding = 0) buffer sharedBufferBoids {
    s_hash htable[BUCKETS_COUNT];
    float boidsCount;
    float windowWidth;
    float windowHeight;
    float bufferSelector;
    s_boid boids[];
};

void checkBorder(const int thisBoid)
{
    if (boids[thisBoid].centerX > windowWidth)
        boids[thisBoid].centerX = 0;
    if (boids[thisBoid].centerX < 0)
        boids[thisBoid].centerX = windowWidth;

    if (boids[thisBoid].centerY > windowHeight)
        boids[thisBoid].centerY = 0;
    if (boids[thisBoid].centerY < 0)
        boids[thisBoid].centerY = windowHeight;
}

float magnitudeVec2(const vec2 a, const vec2 b)
{
    return sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2));
}

vec2 getBoidDirection(const float angleDeg, const float len) {
    float angleRad = angleDeg * PI / 180;
    return vec2(len * cos(angleRad), len * sin(angleRad));
}

// TODO
// void checkAllNeighbours()
// {
// }

bool intersects(vec2 A, vec2 B, vec2 C, vec2 D) {
    float det, gamma, lambda;
    det = (B.x - A.x) * (D.y - C.y) - (D.x - C.x) * (B.y - A.y);
    if (det == 0)
        return false;
    else {
        lambda = ((D.y - C.y) * (D.x - A.x) + (C.x - D.x) * (D.y - A.y)) / det;
        gamma = ((A.y - B.y) * (D.x - A.x) + (B.x - A.x) * (D.y - A.y)) / det;
        return (0 < lambda && lambda < 1) && (0 < gamma && gamma < 1);
    }
};

vec2 normalizeVec2(const vec2 a, const vec2 b)
{
    float magnitude = magnitudeVec2(a, b);
    vec2 normalize = vec2(a.x - b.x, a.y - b.y);

    normalize.x /= magnitude;
    normalize.y /= magnitude;

    return normalize;
}

vec2 rotatePointAroundCenter(vec2 point, const vec2 center, const float angleDeg)
{
    float angleRad = angleDeg * PI / 180;

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

int mappedRadToDeg(float rad)
{
    float deg = rad * 180.0 / PI;
    return int(deg + 360) % 360;
}

int mappedDegrees(int deg)
{
    return int(deg + 360) % 360;
}

void processRotation(int resultAngle, int thisBoidAngle, int thisBoid)
{
    int degreesDif = resultAngle - thisBoidAngle;

    if (abs(degreesDif) <= ROTATION_SPEED) return;
    int tmpRotationSpeed = ROTATION_SPEED;
    if (resultAngle < thisBoidAngle && thisBoidAngle - resultAngle <= 180) tmpRotationSpeed *= -1;
    if (resultAngle > thisBoidAngle && resultAngle- thisBoidAngle > 180) tmpRotationSpeed *= -1;

    boids[thisBoid].angleDeg += tmpRotationSpeed;
}

bool raycast(vec2 directionVec, int thisBoid)
{
    // if (boids[thisBoid].centerX > windowWidth || boids[thisBoid].centerX < 0 || boids[thisBoid].centerY > windowHeight || boids[thisBoid].centerY < 0) {
    // if (boids[thisBoid].centerX > windowWidth / 2 || boids[thisBoid].centerX < windowWidth / 2 || boids[thisBoid].centerY > windowHeight / 2 || boids[thisBoid].centerY < windowHeight / 2 ) {
    if (boids[thisBoid].centerY > windowHeight || boids[thisBoid].centerY < 0) {
        vec2 mapCenter = vec2(windowWidth / 2, windowHeight / 2);
        int resultInDegrees = mappedDegrees(int(atan(mapCenter.y - boids[thisBoid].centerY, mapCenter.x - boids[thisBoid].centerX) * 180 / PI));
        // processRotation(resultInDegrees, mappedDegrees(int(boids[thisBoid].angleDeg - 90)), thisBoid);
        boids[thisBoid].angleDeg = resultInDegrees + 90;
        return true;
    }

    const int size = 4;
    const float offset = 500;
    vec4 walls[size];
    walls[0] = vec4(0 - offset, 0, windowWidth + offset, 0);
    // walls[1] = vec4(windowWidth, 0 - offset, windowWidth, windowHeight + offset);
    walls[2] = vec4(windowWidth + offset, windowHeight, 0 - offset, windowHeight);
    // walls[3] = vec4(0, windowHeight + offset, 0, 0 - offset);

    float raycastAngle = 2.0f;
    vec2 center = vec2(boids[thisBoid].centerX, boids[thisBoid].centerY);
    vec2 frontRay = center + directionVec * RAYCAST_RANGE;
    vec2 leftRay = frontRay;
    vec2 rightRay = frontRay;
    rightRay = rotatePointAroundCenter(rightRay, center, raycastAngle);
    leftRay = rotatePointAroundCenter(leftRay, center, -raycastAngle);
    bool isColliding = false;
    for (int i = 0; i < size; i++) {
        if (intersects(center, frontRay, walls[i].xy, walls[i].zw)
        || intersects(center, leftRay, walls[i].xy, walls[i].zw)
        || intersects(center, rightRay, walls[i].xy, walls[i].zw)) {
            isColliding = true;
            break;
        }
    }
    if (!isColliding) return false;

    int collisionL = 0;
    int collisionR = 0;
    for (float it = raycastAngle; it <= 180.0f; it += raycastAngle) {
        collisionL = 0;
        collisionR = 0;
        rightRay = rotatePointAroundCenter(rightRay, center, raycastAngle);
        leftRay = rotatePointAroundCenter(leftRay, center, -raycastAngle);
        for (int i = 0; i < size; i++) {
            if (intersects(center, rightRay, walls[i].xy, walls[i].zw))
                collisionR++;
            if (intersects(center, leftRay, walls[i].xy, walls[i].zw))
                collisionL++;
        }
        if (collisionR == 0) {
            boids[thisBoid].angleDeg += ROTATION_SPEED * 2;
            return true;
        }
        if (collisionL == 0) {
            boids[thisBoid].angleDeg -= ROTATION_SPEED * 2;
            return true;
        }
    }
    return true;
}

int findClosestInRangeAngle(float range, int sepCount, int cohCount, int aliCount, vec2 separationVec, vec2 cohesionVec, vec2 alignementVec)
{
    if (sepCount != 0 && SEPARATION == range) {
        return mappedDegrees(int(atan(separationVec.y, separationVec.x) * 180 / PI));
    } else if (cohCount != 0 && COHESION == range) {
        return mappedDegrees(int(atan(cohesionVec.y, cohesionVec.x) * 180 / PI));
    } else if (aliCount != 0 && ALIGNMENT == range) {
        return mappedDegrees(int(atan(alignementVec.y, alignementVec.x) * 180 / PI));
    }
    return 0;
}

void main()
{
    if (gl_GlobalInvocationID.x >= boidsCount) return;

    int thisBoid = int(gl_GlobalInvocationID.x);
    if (bufferSelector == 2.0f) thisBoid += int(boidsCount);

    int startIdx = int(htable[int(boids[gl_GlobalInvocationID.x].hashKey)].startIdx);
    if (bufferSelector == 2.0f) startIdx += int(boidsCount);
    int maxIdx = startIdx + int(htable[int(boids[gl_GlobalInvocationID.x].hashKey)].size);

    float maxRange = COHESION > ALIGNMENT ? COHESION > SEPARATION ? COHESION : SEPARATION : ALIGNMENT > SEPARATION ? ALIGNMENT : SEPARATION;
    float minRange = COHESION < ALIGNMENT ? COHESION < SEPARATION ? COHESION : SEPARATION : ALIGNMENT < SEPARATION ? ALIGNMENT : SEPARATION;
    float midRange = 0.0f;
    if (COHESION != maxRange && COHESION != minRange) midRange = COHESION;
    else if (ALIGNMENT != maxRange && ALIGNMENT != minRange) midRange = ALIGNMENT;
    else midRange = SEPARATION;

    vec2 separationVec = vec2(0, 0);
    int sepCount = 0;
    vec2 cohesionVec = vec2(0, 0);
    int cohCount = 0;
    vec2 alignementVec = vec2(0, 0);
    int aliCount = 0;

    float magnitude = 0.0f;
    for (int i = startIdx; i < maxIdx; i++) {
        if (i == thisBoid) continue;
        magnitude = magnitudeVec2(vec2(boids[i].centerX, boids[i].centerY), vec2(boids[thisBoid].centerX, boids[thisBoid].centerY));
        if (mappedDegrees(int(boids[i].angleDeg - 90)) - mappedDegrees(int(boids[thisBoid].angleDeg - 90)) > FOV / 2 && mappedDegrees(int(boids[i].angleDeg - 90)) - mappedDegrees(int(boids[thisBoid].angleDeg - 90)) < 360 - FOV / 2)
            continue;
        if (magnitude < SEPARATION) {
            separationVec.x += boids[i].centerX;
            separationVec.y += boids[i].centerY;
            sepCount++;
        }
        if (magnitude < COHESION) {
            cohesionVec.x += boids[i].centerX;
            cohesionVec.y += boids[i].centerY;
            cohCount++;
        }
        if (magnitude < ALIGNMENT) {
            alignementVec += getBoidDirection(boids[i].angleDeg - 90, 1.0f);
            aliCount++;
        }
    }
    if (sepCount != 0) {
        separationVec.x = separationVec.x / sepCount - boids[thisBoid].centerX;
        separationVec.y = separationVec.y / sepCount - boids[thisBoid].centerY;
        separationVec.x *= -1;
        separationVec.y *= -1;
    }
    if (cohCount != 0) {
        cohesionVec.x = cohesionVec.x / cohCount - boids[thisBoid].centerX;
        cohesionVec.y = cohesionVec.y / cohCount - boids[thisBoid].centerY;
    }
    if (aliCount != 0) {
        alignementVec.x /= aliCount;
        alignementVec.y /= aliCount;
    }

    vec2 directionVec = getBoidDirection(boids[thisBoid].angleDeg - 90, 1.0f);
    raycast(directionVec, thisBoid);
    // if (!raycast(directionVec, thisBoid)) {
    if (true) {
        int resultInDegrees = findClosestInRangeAngle(minRange, sepCount, cohCount, aliCount, separationVec, cohesionVec, alignementVec);
        if (resultInDegrees == 0)
            resultInDegrees = findClosestInRangeAngle(midRange, sepCount, cohCount, aliCount, separationVec, cohesionVec, alignementVec);
        if (resultInDegrees == 0)
           resultInDegrees = findClosestInRangeAngle(maxRange, sepCount, cohCount, aliCount, separationVec, cohesionVec, alignementVec);
        if (resultInDegrees != 0)
            processRotation(resultInDegrees, mappedDegrees(int(boids[thisBoid].angleDeg - 90)), thisBoid);
    }
    directionVec = getBoidDirection(boids[thisBoid].angleDeg - 90, 1.0f);
    boids[thisBoid].centerX += directionVec.x * SPEED;
    boids[thisBoid].centerY += directionVec.y * SPEED;
    checkBorder(thisBoid);
}