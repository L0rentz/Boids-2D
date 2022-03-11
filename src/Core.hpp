#ifndef CORE_HPP_
#define CORE_HPP_

#include "Boid.hpp"

#include <SFML/Graphics.hpp>
#include <time.h>

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
        bool _leftMouseClick;
};

#endif /* !CORE_HPP_ */
