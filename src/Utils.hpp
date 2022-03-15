#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <SFML/Graphics.hpp>
#include <cmath>

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

class Utils {
    public:
        Utils();
        ~Utils();

        float angleBetweenVector2f(const glm::vec2 a, const glm::vec2 b);
        float magnitudeVector2f(const glm::vec2 a, const glm::vec2 b);
        float dotProductVector2f(const glm::vec2 a, const glm::vec2 b);
        glm::vec2 rotatePointAroundCenter(glm::vec2 point, const glm::vec2 center, const float degrees);
        glm::vec2 normalizeVector2f(const glm::vec2 a, const glm::vec2 b);
        void posDebug(const glm::vec2 position, sf::RenderWindow &window);
        unsigned int mappedDegreesAngleDif(const glm::vec2 a, const glm::vec2 tail, const glm::vec2 b);
        unsigned int mappedDegrees(float radians);
        bool segmentIntersectsRectangle(const sf::FloatRect &rect, const glm::vec2 &a_p1, const glm::vec2 &a_p2);

    protected:
    private:
};

#endif /* !UTILS_HPP_ */
