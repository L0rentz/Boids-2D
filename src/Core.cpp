#include "Core.hpp"
#include "glad/glad.h"

Core::Core()
{
    // srand(static_cast<unsigned int>(time(NULL)));
    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;
    _window.create(sf::VideoMode(1920, 1080), "Boids simulation", sf::Style::Default, settings);
    // _window.setFramerateLimit(60);

    if(!gladLoadGL()) throw Exception("gladLoadGL failed");

    _boids = new Boid[BOIDS_COUNT];
    for (int i = 0; i < BOIDS_COUNT; i++) {
        int x = static_cast<int>(WALLOFFSET + rand() % static_cast<int>(_window.getSize().x - WALLOFFSET * 2));
        int y = static_cast<int>(WALLOFFSET + rand() % static_cast<int>(_window.getSize().y - WALLOFFSET * 2));
        _boids[i] = Boid(glm::vec2{x, y}, static_cast<float>(_window.getSize().x), 4, 4, 4);
    }

    _offset = 5;
    _arraySize = BOIDS_COUNT * _offset + 1;
    _worldPosScaleAngleDeg = new float[_arraySize];
    _worldPosScaleAngleDeg[0] = static_cast<float>(_arraySize);
    for (unsigned int i = 0, offset = 1; i < BOIDS_COUNT; i++, offset += _offset) {
        _worldPosScaleAngleDeg[0 + offset] = _boids[i]._center.x;
        _worldPosScaleAngleDeg[1 + offset] = _boids[i]._center.y;
        _worldPosScaleAngleDeg[2 + offset] = _boids[i]._scale.x;
        _worldPosScaleAngleDeg[3 + offset] = _boids[i]._scale.y;
        _worldPosScaleAngleDeg[4 + offset] = static_cast<float>(_boids[i]._angleDeg);
    }

    openGlInit();

    _running = true;
    _wireframe = false;
    _runTime.restart();
}

Core::~Core()
{
    delete[] _worldPosScaleAngleDeg;
    delete[] _boids;
    // for (auto it : _boids)
    //     delete it;
    // for (auto it : _obstacles)
    //     delete it;
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(_computeProgram);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _SSBO);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * _arraySize, &_worldPosScaleAngleDeg[0], GL_DYNAMIC_DRAW);
            glDispatchCompute(static_cast<GLuint>(glm::ceil(_arraySize / static_cast<float>(_offset) / 32.0f)), 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            _worldPosScaleAngleDeg = (float *)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    glUseProgram(0);

    glUseProgram(_vertexFragProgram);
    glUniformMatrix4fv(glGetUniformLocation(_vertexFragProgram, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));

    glBindBuffer(GL_ARRAY_BUFFER, _instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, (sizeof(glm::vec4) + sizeof(float)) * BOIDS_COUNT, &_worldPosScaleAngleDeg[1], GL_DYNAMIC_DRAW);
    Boid::prepareDrawingBuffers(_VAO, _VBO, _instanceVBO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, BOIDS_COUNT);
    Boid::clearDrawingBuffers(_VAO);
    glUseProgram(0);

    _window.display();
}

// void Core::update()
// {
//     _hashTable.clear();
//     for (auto it : _boids)
//         _hashTable[it->getGridID()].push_back(it);
//     for (auto it : _boids)
//         it->update(_window.getSize(), _hashTable, _obstacles);
// }

const char *Core::getFileContent(const std::string& path) const
{
    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    char *cstr = new char[content.size() + 1];
    cstr = std::strcpy(cstr, content.c_str());

    return cstr;
}

void Core::checkShaderCompileError(unsigned int shaderId)
{
    int  success;
    char infoLog[512];
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
        std::string errorMsg = "ERROR::SHADER::COMPILATION_FAILED\n";
        errorMsg += infoLog;
        throw Exception(errorMsg);
    }
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
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH);

    const char *vertexShaderSource = getFileContent(std::string(SHADER_PATH) + "boid.vs");
    _vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(_vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(_vertexShader);
    checkShaderCompileError(_vertexShader);

    const char *fragmentShaderSource = getFileContent(std::string(SHADER_PATH) + "boid.fs");
    _fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(_fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(_fragmentShader);
    checkShaderCompileError(_fragmentShader);

    _vertexFragProgram = glCreateProgram();
    glAttachShader(_vertexFragProgram, _vertexShader);
    glAttachShader(_vertexFragProgram, _fragmentShader);
    glLinkProgram(_vertexFragProgram);
    checkShaderProgramCompileError(_vertexFragProgram);

    const char *computeShaderSource = getFileContent(std::string(SHADER_PATH) + "boid.comp");
    _computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(_computeShader, 1, &computeShaderSource, NULL);
    glCompileShader(_computeShader);
    checkShaderCompileError(_computeShader);

    _computeProgram = glCreateProgram();
    glAttachShader(_computeProgram, _computeShader);
    glLinkProgram(_computeProgram);
    checkShaderProgramCompileError(_computeProgram);

    glDeleteShader(_vertexShader);
    delete[] vertexShaderSource;
    glDeleteShader(_fragmentShader);
    delete[] fragmentShaderSource;
    glDeleteShader(_computeShader);
    delete[] computeShaderSource;

    glGenBuffers(1, &_VBO);
    glGenBuffers(1, &_instanceVBO);
    glGenVertexArrays(1, &_VAO);

    glGenBuffers(1, &_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _SSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _SSBO);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * _arraySize, &_worldPosScaleAngleDeg[0], GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    _projection = glm::mat4(1.0f);
    _projection = glm::ortho(0.0f, (float)_window.getSize().x, (float)_window.getSize().y, 0.0f, -1.0f, 1.0f);
}

void Core::run()
{
    double lastTime = _runTime.getElapsedTime().asSeconds();
    int nbFrames = 0;

    while (_running) {
        double currentTime = _runTime.getElapsedTime().asSeconds();
        nbFrames++;
        if (currentTime - lastTime >= 1.0) {
            printf("%d fps\n", nbFrames);
            nbFrames = 0;
            lastTime += 1.0;
        }
        events();
        // update();
        display();
    }
    glDeleteBuffers(1, &_SSBO);
    _window.close();
}