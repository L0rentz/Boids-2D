#ifndef CORE_HPP_
#define CORE_HPP_

#include "stb_image.h"
#include "Boid.hpp"
#include "Exception.hpp"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <SFML/Graphics.hpp>
#include <time.h>
#include <map>
#include <fstream>
#include <cstring>
#include <thread>

#ifdef _WIN32
    #define SHADER_PATH "../../src/"
#else
    #define SHADER_PATH "src/"
#endif

class Core {
    public:
        Core();
        ~Core();

        void run();

    protected:
    private:
        void events();
        void display();
        void update();

        sf::Window _window;
        sf::Event _event;
        Boid *_boids;
        float *_worldPosScaleAngleDeg;
        unsigned int _offset;
        unsigned int _metadataSize;
        int _arraySize;
        std::map<int, std::vector<Boid *>> _hashTable;
        bool _running;
        sf::Clock _runTime;
        double _lastTime;
        double _currentTime;
        int _framerate;

        // For OpenGL

        const char *getFileContent(const std::string &path) const;
        void compileShader(unsigned int *shaderId, std::string filename, unsigned int type);
        void checkShaderProgramCompileError(unsigned int shaderProgramId);
        void openGlInit();
        void openGlDraw();

        bool _wireframe;
        unsigned int _VAO, _VBO, _instanceVBO, _SSBO;
        unsigned int _vertexFragProgram;
        unsigned int _computeProgram;
        unsigned int _vertexShader;
        unsigned int _computeShader;
        unsigned int _fragmentShader;
        unsigned int _texture[2];
        glm::mat4 _projection;

};

#endif /* !CORE_HPP_ */
