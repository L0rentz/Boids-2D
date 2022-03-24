#version 460 core

#define BUCKETS_COUNT 400

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

void main()
{
    if (gl_GlobalInvocationID.x >= BUCKETS_COUNT) return;

    int oldBoidsBuffer = bufferSelector == 2.0f ? 0 : int(boidsCount);
    int newBoidsBuffer = bufferSelector == 2.0f ? int(boidsCount) : 0;

    for (int i = 0, sortIdx = int(htable[gl_GlobalInvocationID.x].startIdx); i < boidsCount; i++, oldBoidsBuffer++) {
        if (boids[oldBoidsBuffer].hashKey == gl_GlobalInvocationID.x) {
            boids[newBoidsBuffer + sortIdx] = boids[oldBoidsBuffer];
            sortIdx++;
        }
    }
}