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

    // game.run();
    game.run();
    return EXIT_SUCCESS;
}
