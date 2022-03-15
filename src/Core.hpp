#ifndef CORE_HPP_
#define CORE_HPP_

#include "stb_image.h"
#include "Boid.hpp"
#include "glad/glad.h"
#include "Exception.hpp"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <time.h>
#include <map>
#include <fstream>
#include <cstring>

#ifdef _WIN32
    #define SHADER_PATH "../src/"
#else
    #define SHADER_PATH "src/"
#endif

#define BOIDS_COUNT 500000

class Core {
    public:
        Core(const std::string &title = "Boids simulation", unsigned int framerate = 60);
        ~Core();

        void run();

    protected:
    private:
        void events();
        void display();
        void update();
        void generateBorders();
        void drawObstacles();
        void createObstacleOnClick();

        sf::RenderWindow _window;
        sf::Event _event;
        std::vector<Boid *> _boids;
        std::vector<sf::FloatRect *> _obstacles;
        sf::FloatRect *_placeHolder;
        std::map<int, std::vector<Boid *>> _hashTable;
        bool _leftMouseClick;
        bool _running;
        sf::Clock _runTime;

        // For OpenGL

        const char *getFileContent(const std::string &path) const;
        void checkShaderCompileError(unsigned int shaderId);
        void checkShaderProgramCompileError(unsigned int shaderProgramId);
        void openGlInit();
        void openGlDraw();

        bool _wireframe;
        unsigned int _VAO, _VBO, _instanceVBO;
        unsigned int _shaderProgram;
        unsigned int _vertexShader;
        unsigned int _fragmentShader;
        unsigned int _texture[2];
        glm::mat4 _projection;
        glm::mat4 _model;
};

#endif /* !CORE_HPP_ */
