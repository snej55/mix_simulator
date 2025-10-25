//
// Created by Jens Kromdijk on 07/04/25.
//

#ifndef UTIL_H
#define UTIL_H

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

namespace Util
{
    // Generate random floating point from (0.0f - 1.0f).
    inline float random() { return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX); }

    // Checks if file exists in file system.
    inline bool fileExists(const std::string& name)
    {
        std::ifstream file{name.c_str()};
        return file.good();
    }

    // Render text with red background.
    inline void beginError() { std::cout << "\033[41m"; }

    // Reset texture color.
    inline void endError() { std::cout << "\033[m" << std::endl; }

    inline float lerp(const float a, const float b, const float amount) { return a + (b - a) * amount; }
} // namespace Util

#endif // UTIL_H
