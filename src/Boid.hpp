#ifndef BOID_HPP_
#define BOID_HPP_

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#define WALLOFFSET -1.0

#define BOIDS_COUNT 50000
#define BUCKETS_COUNT 100 // Change in both GLSL compute shader too !

class Boid {
    public:
        Boid();
        Boid(const glm::vec2 position, const float screenWidth, const int scale = 20);
        ~Boid();

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
        static float _screenWidth;
        static int _nextID;

        static int _cellWidth;
        static int _gridWidth;
        int _diameter;
        int _radius;
        int _id;

        void setVerticeModel(float x, float y, unsigned int i);
};

#endif /* !BOID_HPP_ */
