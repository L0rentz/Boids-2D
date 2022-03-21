#include "Core.hpp"
#include "glad/glad.h"

Core::Core()
{
    srand(static_cast<unsigned int>(time(NULL)));
    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;
    _window.create(sf::VideoMode(1920, 1080), "Boids simulation", sf::Style::Default, settings);
    _framerate = 60;
    _window.setFramerateLimit(_framerate);
    _running = true;
    _runTime.restart();
    _lastTime = _runTime.getElapsedTime().asSeconds();

    _boids = new Boid[BOIDS_COUNT];
    for (int i = 0; i < BOIDS_COUNT; i++) {
        int x = static_cast<int>(WALLOFFSET + rand() % static_cast<int>(_window.getSize().x - WALLOFFSET * 2));
        int y = static_cast<int>(WALLOFFSET + rand() % static_cast<int>(_window.getSize().y - WALLOFFSET * 2));
        _boids[i] = Boid(glm::vec2{x, y}, static_cast<float>(_window.getSize().x), 2, 4, 4);
    }

    _tableSize = BUCKETS_COUNT * 2;
    _metadataSize = 4;
    _worldPosScaleAngleDegOffset = 6;
    _worldPosScaleAngleDegSize = BOIDS_COUNT * _worldPosScaleAngleDegOffset;
    _worldPosScaleAngleDegIdx1 = _tableSize + _metadataSize;
    _worldPosScaleAngleDegIdx2 = _tableSize + _metadataSize + _worldPosScaleAngleDegSize;
    _bufferSize = _tableSize + _metadataSize + _worldPosScaleAngleDegSize * 2;
    _sharedBuffer = new float[_bufferSize];
    std::memset(_sharedBuffer, 0, _bufferSize * sizeof(float));
    _sharedBuffer[_tableSize] = static_cast<float>(BOIDS_COUNT);
    _sharedBuffer[_tableSize + 1] = static_cast<float>(_window.getSize().x);
    _sharedBuffer[_tableSize + 2] = static_cast<float>(_window.getSize().y);
    _bufferSelectorIdx = _tableSize + 3;
    _sharedBuffer[_bufferSelectorIdx] = 1.0f;
    for (unsigned int j = 0, inc = _tableSize + _metadataSize; j < 2; j++) {
        for (unsigned int i = 0; i < BOIDS_COUNT; i++, inc += _worldPosScaleAngleDegOffset) {
            _sharedBuffer[inc] = _boids[i].center.x;
            _sharedBuffer[inc + 1] = _boids[i].center.y;
            _sharedBuffer[inc + 2] = _boids[i].scale.x;
            _sharedBuffer[inc + 3] = _boids[i].scale.y;
            _sharedBuffer[inc + 4] = static_cast<float>(_boids[i].angleDeg);
            _sharedBuffer[inc + 5] = 0.0f; // hashKey
        }
    }

    openGlInit();
}

Core::~Core()
{
    delete[] _boids;
}

const char *Core::getFileContent(const std::string& path) const
{
    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    char *cstr = new char[content.size() + 1];
    cstr = std::strcpy(cstr, content.c_str());

    return cstr;
}

void Core::compileShader(unsigned int *shaderId, std::string filename, unsigned int type)
{
    const char *shaderSource = getFileContent(std::string(SHADER_PATH) + filename);
    *shaderId = glCreateShader(type);
    glShaderSource(*shaderId, 1, &shaderSource, NULL);
    glCompileShader(*shaderId);

    int success;
    char infoLog[512];
    glGetShaderiv(*shaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(*shaderId, 512, NULL, infoLog);
        std::string errorMsg = "ERROR::SHADER::COMPILATION_FAILED\n";
        errorMsg += infoLog;
        throw Exception(errorMsg);
    }

    delete[] shaderSource;
}

void Core::checkShaderProgramCompileError(unsigned int shaderProgramId)
{
    int  success;
    char infoLog[512];
    glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgramId, 512, NULL, infoLog);
        std::string errorMsg = "ERROR::SHADER::COMPILATION_FAILED\n";
        errorMsg += infoLog;
        throw Exception(errorMsg);
    }
}

void Core::openGlInit()
{
    if(!gladLoadGL()) throw Exception("gladLoadGL failed");

    _wireframe = false;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    compileShader(&_vertexShader, "boid.vs", GL_VERTEX_SHADER);
    compileShader(&_fragmentShader, "boid.fs", GL_FRAGMENT_SHADER);
    _vertexFragProgram = glCreateProgram();
    glAttachShader(_vertexFragProgram, _vertexShader);
    glAttachShader(_vertexFragProgram, _fragmentShader);
    glLinkProgram(_vertexFragProgram);
    checkShaderProgramCompileError(_vertexFragProgram);
    glDeleteShader(_vertexShader);
    glDeleteShader(_fragmentShader);

    compileShader(&_computeShader, "boid_flocking.comp", GL_COMPUTE_SHADER);
    _computeProgramFlocking = glCreateProgram();
    glAttachShader(_computeProgramFlocking, _computeShader);
    glLinkProgram(_computeProgramFlocking);
    checkShaderProgramCompileError(_computeProgramFlocking);
    glDeleteShader(_computeShader);

    compileShader(&_computeShader, "boid_hashing.comp", GL_COMPUTE_SHADER);
    _computeProgramHashing = glCreateProgram();
    glAttachShader(_computeProgramHashing, _computeShader);
    glLinkProgram(_computeProgramHashing);
    checkShaderProgramCompileError(_computeProgramHashing);
    glDeleteShader(_computeShader);

    glGenBuffers(1, &_VBO);
    glGenBuffers(1, &_instanceVBO);
    glGenBuffers(1, &_SSBO);
    glGenVertexArrays(1, &_VAO);

    _projection = glm::mat4(1.0f);
    _projection = glm::ortho(0.0f, (float)_window.getSize().x, (float)_window.getSize().y, 0.0f, -1.0f, 1.0f);
}

void Core::events()
{
    while (_window.pollEvent(_event)) {
        if (_event.type == sf::Event::Closed)
            _running = false;
        if (_event.type == sf::Event::KeyPressed) {
            if (_event.key.code == sf::Keyboard::Space) {
                _wireframe ? glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) : glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                _wireframe = !_wireframe;
            }
        }
    }
}

void Core::display()
{
    Boid::updateHashtable(_sharedBuffer, _tableSize, _sharedBuffer[_bufferSelectorIdx] == 1.0f ? &_sharedBuffer[_worldPosScaleAngleDegIdx1] : &_sharedBuffer[_worldPosScaleAngleDegIdx2], _worldPosScaleAngleDegOffset, &_sharedBuffer[_bufferSelectorIdx]);

    static int firstIt = 0;
    glUseProgram(_computeProgramHashing);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _SSBO);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * _bufferSize, &_sharedBuffer[0], GL_STREAM_DRAW);
            if (!firstIt) {
                delete[] _sharedBuffer;
                firstIt++;
            }
            glDispatchCompute(static_cast<GLuint>(glm::ceil(BUCKETS_COUNT / 32.0f)), 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            _sharedBuffer = (float *)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    glUseProgram(0);

    glUseProgram(_computeProgramFlocking);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _SSBO);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * _bufferSize, &_sharedBuffer[0], GL_STREAM_DRAW);
            glDispatchCompute(static_cast<GLuint>(glm::ceil(BOIDS_COUNT / 32.0f)), 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            _sharedBuffer = (float *)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    glUseProgram(0);

    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(_vertexFragProgram);
        glUniformMatrix4fv(glGetUniformLocation(_vertexFragProgram, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));
        Boid::prepareDrawingBuffers(_VAO, _VBO, _instanceVBO, _sharedBuffer[_bufferSelectorIdx] == 2.0f ? &_sharedBuffer[_worldPosScaleAngleDegIdx2] : &_sharedBuffer[_worldPosScaleAngleDegIdx1]);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 3, BOIDS_COUNT);
        Boid::clearDrawingBuffers(_VAO);
    glUseProgram(0);

    _window.display();
}

void Core::run()
{
    double lastTime = _runTime.getElapsedTime().asSeconds();
    int nbFrames = 0;

    while (_running) {
        double currentTime = _runTime.getElapsedTime().asSeconds();
        nbFrames++;
        if (currentTime - lastTime >= 1.0) {
            std::cout << nbFrames << " fps" << std::endl;
            nbFrames = 0;
            lastTime += 1.0;
        }
        events();
        display();
    }

    glDeleteBuffers(1, &_VBO);
    glDeleteBuffers(1, &_instanceVBO);
    glDeleteBuffers(1, &_SSBO);

    _window.close();
}