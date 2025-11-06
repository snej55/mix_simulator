//
// Created by Jens Kromdijk on 07/04/25.
//

#ifndef UTIL_H
#define UTIL_H

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <assimp/matrix4x4.h>
#include <glm/glm.hpp>

namespace Util
{
    // Generate random floating point from (0.0f - 1.0f).
    inline float random() { return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX); }

    // Checks if file exists in file system.
    inline bool fileExists(const std::string& name)
    {
        const std::ifstream file{name.c_str()};
        return file.good();
    }

    // Render text with red background.
    inline void beginError() { std::cout << "\033[41m"; }

    // Reset texture color.
    inline void endError() { std::cout << "\033[m" << std::endl; }

    inline float lerp(const float a, const float b, const float amount) { return a + (b - a) * amount; }

    // convert mat4 from assimp format to glm format
    inline glm::mat4 convertMatrixGLM(const aiMatrix4x4& mat)
    {
        const glm::mat4 glmMat {
            {mat.a1, mat.a2, mat.a3, mat.a4},
            {mat.b1, mat.b2, mat.b3, mat.b4},
            {mat.c1, mat.c2, mat.c3, mat.c4},
            {mat.d1, mat.d2, mat.d3, mat.d4},
        };
        return glmMat;
    }
} // namespace Util

#endif // UTIL_H
