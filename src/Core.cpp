#include "Core.hpp"
#include "glad/glad.h"

Core::Core()
{
    srand(static_cast<unsigned int>(time(NULL)));
    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;
    _window.create(sf::VideoMode(1920, 1080), "Boids simulation", sf::Style::Default, settings);
    _framerate = 60;
    // _window.setFramerateLimit(_framerate);

    if(!gladLoadGL()) throw Exception("gladLoadGL failed");

    _boids = new Boid[BOIDS_COUNT];
    for (int i = 0; i < BOIDS_COUNT; i++) {
        int x = static_cast<int>(WALLOFFSET + rand() % static_cast<int>(_window.getSize().x - WALLOFFSET * 2));
        int y = static_cast<int>(WALLOFFSET + rand() % static_cast<int>(_window.getSize().y - WALLOFFSET * 2));
        _boids[i] = Boid(glm::vec2{x, y}, static_cast<float>(_window.getSize().x), 1, 4, 4);
    }

    _offset = 5;
    _metadataSize = 3;
    _arraySize = BOIDS_COUNT * _offset + _metadataSize;
    _worldPosScaleAngleDeg = new float[_arraySize];
    _worldPosScaleAngleDeg[0] = static_cast<float>(BOIDS_COUNT);
    _worldPosScaleAngleDeg[1] = static_cast<float>(_window.getSize().x);
    _worldPosScaleAngleDeg[2] = static_cast<float>(_window.getSize().y);
    for (unsigned int i = 0, offset = _metadataSize; i < BOIDS_COUNT; i++, offset += _offset) {
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
    _lastTime = _runTime.getElapsedTime().asSeconds();
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
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    compileShader(&_vertexShader, "boid.vs", GL_VERTEX_SHADER);
    compileShader(&_fragmentShader, "boid.fs", GL_FRAGMENT_SHADER);
    _vertexFragProgram = glCreateProgram();
    glAttachShader(_vertexFragProgram, _vertexShader);
    glAttachShader(_vertexFragProgram, _fragmentShader);
    glLinkProgram(_vertexFragProgram);
    checkShaderProgramCompileError(_vertexFragProgram);
    glDeleteShader(_vertexShader);
    glDeleteShader(_fragmentShader);

    compileShader(&_computeShader, "boid.comp", GL_COMPUTE_SHADER);
    _computeProgram = glCreateProgram();
    glAttachShader(_computeProgram, _computeShader);
    glLinkProgram(_computeProgram);
    checkShaderProgramCompileError(_computeProgram);
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
    static int firstIt = 0;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    _currentTime = _runTime.getElapsedTime().asSeconds();
    if (_currentTime - _lastTime >= 1.0f / _framerate) {
        glUseProgram(_computeProgram);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _SSBO);
                glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * _arraySize, &_worldPosScaleAngleDeg[0], GL_STREAM_DRAW);
                if (!firstIt) {
                    delete[] _worldPosScaleAngleDeg;
                    firstIt++;
                }
                glDispatchCompute(static_cast<GLuint>(glm::ceil(_arraySize / static_cast<float>(_offset) / 32.0f)), 1, 1);
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                _worldPosScaleAngleDeg = (float *)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
                glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
        glUseProgram(0);
        _lastTime += 1.0f / _framerate;
    }

    glUseProgram(_vertexFragProgram);
        glUniformMatrix4fv(glGetUniformLocation(_vertexFragProgram, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));
        Boid::prepareDrawingBuffers(_VAO, _VBO, _instanceVBO, _worldPosScaleAngleDeg, _metadataSize);
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
            printf("%d fps\n", nbFrames);
            nbFrames = 0;
            lastTime += 1.0;
        }
        events();
        display();
    }

    glDeleteBuffers(1, &_SSBO);
    _window.close();
}