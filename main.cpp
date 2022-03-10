#include "Exception.hpp"
#include "Core.hpp"

int main(void)
{
    Core core;
    try {
        core.run();
    } catch (Exception &e) {
        std::cout << e.what() << std::endl;
        return 84;
    }
    return 0;
}