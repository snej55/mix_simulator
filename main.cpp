
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include "src/game.hpp"

#include <cstdlib>

int main()
{
    Game game{};
    if (!game.init())
    {
        std::cerr << "ERROR: Failed to initialize game!" << std::endl;
        return EXIT_FAILURE;
    }

    while (game.menu())
    {
        game.run();
        if (!game.gameover())
            break;
    }
    return EXIT_SUCCESS;
}
