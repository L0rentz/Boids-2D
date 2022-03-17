#include "Core.hpp"
#include "glad/glad.h"

Core::Core()
{
    srand(static_cast<unsigned int>(time(NULL)));
    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;
    _window.create(sf::VideoMode(1920, 1080), "Boids simulation", sf::Style::Default, settings);
    // _window.setFramerateLimit(60);

    if(!gladLoadGL()) throw Exception("gladLoadGL failed");

    generateBorders();
    for (int i = 0; i < BOIDS_COUNT; i++) {
        int x = static_cast<int>(WALLOFFSET + rand() % static_cast<int>(_window.getSize().x - WALLOFFSET * 2));
        int y = static_cast<int>(WALLOFFSET + rand() % static_cast<int>(_window.getSize().y - WALLOFFSET * 2));
        _boids.push_back(new Boid(glm::vec2{x, y}, static_cast<float>(_window.getSize().x), 6, 4, 4));
    }

    _worldPosScaleAngleDeg = new float[BOIDS_COUNT][5];

    openGlInit();

    _placeHolder = nullptr;
    _leftMouseClick = false;
    _running = true;
    _wireframe = false;
    _runTime.restart();
}

Core::~Core()
{
    delete[] _worldPosScaleAngleDeg;
    for (auto it : _boids)
        delete it;
    // for (auto it : _obstacles)
    //     delete it;
}

void Core::generateBorders()
{
    _obstacles.push_back(new sf::FloatRect(WALLOFFSET, WALLOFFSET, static_cast<float>(_window.getSize().x - WALLOFFSET * 2), 1));
    _obstacles.push_back(new sf::FloatRect(WALLOFFSET, static_cast<float>(_window.getSize().y - WALLOFFSET), static_cast<float>(_window.getSize().x - WALLOFFSET * 2), 1));
    _obstacles.push_back(new sf::FloatRect(WALLOFFSET, WALLOFFSET, 1, static_cast<float>(_window.getSize().y - WALLOFFSET * 2)));
    _obstacles.push_back(new sf::FloatRect(static_cast<float>(_window.getSize().x - WALLOFFSET), WALLOFFSET, 1, static_cast<float>(_window.getSize().y - WALLOFFSET * 2)));
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
            if (_event.key.code == sf::Keyboard::BackSpace) {
                if (_obstacles.size() > 4) {
                    delete _obstacles.back();
                    _obstacles.pop_back();
                }
            }
        }
        if (_event.type == sf::Event::MouseButtonPressed) {
            if (_event.mouseButton.button == sf::Mouse::Left) {
                _leftMouseClick = true;
                _placeHolder = new sf::FloatRect(static_cast<float>(sf::Mouse::getPosition(_window).x), static_cast<float>(sf::Mouse::getPosition(_window).y), 0, 0);
            }
        }
        if (_event.type == sf::Event::MouseButtonReleased) {
            if (_event.mouseButton.button == sf::Mouse::Left) {
                _leftMouseClick = false;
                if (_placeHolder->height < 0) {
                    _placeHolder->top -= std::abs(_placeHolder->height);
                    _placeHolder->height *= -1;
                }
                else if (_placeHolder->width < 0) {
                    _placeHolder->left -= std::abs(_placeHolder->width);
                    _placeHolder->width *= -1;
                }
                _obstacles.push_back(_placeHolder);
                _placeHolder = nullptr;
            }
        }
    }
}

void Core::drawObstacles()
{
    sf::RectangleShape rectangle;
    for (auto it : _obstacles) {
        rectangle.setPosition(sf::Vector2f{it->left, it->top});
        rectangle.setSize(sf::Vector2f{it->width, it->height});
        _window.draw(rectangle);
    }
    if (_placeHolder) {
        rectangle.setPosition(sf::Vector2f{_placeHolder->left, _placeHolder->top});
        rectangle.setSize(sf::Vector2f{_placeHolder->width, _placeHolder->height});
        _window.draw(rectangle);
    }
}

void Core::display()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(_shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(_shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));

    glm::vec2 worldPos;
    glm::vec2 scale;
    for (unsigned int i = 0; i < BOIDS_COUNT; i++) {
        scale = _boids[i]->getScale();
        worldPos = _boids[i]->getWorldPosition();

        _worldPosScaleAngleDeg[i][0] = worldPos.x;
        _worldPosScaleAngleDeg[i][1] = worldPos.y;
        _worldPosScaleAngleDeg[i][2] = scale.x;
        _worldPosScaleAngleDeg[i][3] = scale.y;
        _worldPosScaleAngleDeg[i][4] = static_cast<float>(_boids[i]->getAngleDeg());
    }

    glBindBuffer(GL_ARRAY_BUFFER, _instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, (sizeof(glm::vec4) + sizeof(float)) * BOIDS_COUNT, &_worldPosScaleAngleDeg[0], GL_STATIC_DRAW);
    Boid::prepareDrawingBuffers(_VAO, _VBO, _instanceVBO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, BOIDS_COUNT);
    Boid::clearDrawingBuffers(_VAO);
    glUseProgram(0);

    drawObstacles();
    _window.display();
}

void Core::update()
{
    _hashTable.clear();
    for (auto it : _boids)
        _hashTable[it->getGridID()].push_back(it);
    for (auto it : _boids)
        it->update(_window.getSize(), _hashTable, _obstacles);
    if (_leftMouseClick) {
        _placeHolder->width = static_cast<float>(sf::Mouse::getPosition(_window).x) - _placeHolder->left;
        _placeHolder->height = static_cast<float>(sf::Mouse::getPosition(_window).y) - _placeHolder->top;
        if (std::abs(_placeHolder->width) > std::abs(_placeHolder->height)) _placeHolder->height = 1.0;
        else _placeHolder->width = 1.0;
    }
}

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

    _shaderProgram = glCreateProgram();
    glAttachShader(_shaderProgram, _vertexShader);
    glAttachShader(_shaderProgram, _fragmentShader);
    glLinkProgram(_shaderProgram);
    checkShaderProgramCompileError(_shaderProgram);

    glDeleteShader(_vertexShader);
    glDeleteShader(_fragmentShader);
    delete[] vertexShaderSource;
    delete[] fragmentShaderSource;

    glGenBuffers(1, &_VBO);
    glGenBuffers(1, &_instanceVBO);
    glGenVertexArrays(1, &_VAO);

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
    _window.close();
}