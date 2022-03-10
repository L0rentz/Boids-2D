#ifndef CORE_HPP_
#define CORE_HPP_

#include "Boid.hpp"

#include <SFML/Graphics.hpp>

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

        sf::RenderWindow _window;
        sf::Event _event;
        std::vector<Boid *> _boids;
};

#endif /* !CORE_HPP_ */
