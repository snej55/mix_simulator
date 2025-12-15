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
#include <assimp/quaternion.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

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

    inline float lerp(const float a, const float b, const float f) { return a + (b - a) * f; }

    // convert mat4 from assimp format to glm format
    inline glm::mat4 convertMatrixGLM(const aiMatrix4x4& mat)
    {
        const glm::mat4 glmMat{
            {mat.a1, mat.b1, mat.c1, mat.d1},
            {mat.a2, mat.b2, mat.c2, mat.d2},
            {mat.a3, mat.b3, mat.c3, mat.d3},
            {mat.a4, mat.b4, mat.c4, mat.d4},
        };
        return glmMat;
    }

    // convert aiVector3D to glm::vec3
    inline glm::vec3 convertVectorGLM(const aiVector3D& vec) { return glm::vec3{vec.x, vec.y, vec.z}; }

    // convert aiQuaternion to glm::quat
    inline glm::quat convertQuaternionGLM(const aiQuaternion& quat)
    {
        return glm::quat{quat.w, quat.x, quat.y, quat.z};
    }

    inline glm::mat4 stripScale(const glm::mat4& mat)
    {
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(mat, scale, rotation, translation, skew, perspective);

        const glm::mat4 rotationMatrix{glm::mat4_cast(rotation)};
        const glm::mat4 translationMatrix{glm::translate(glm::mat4{1.0f}, translation)};
        return translationMatrix * rotationMatrix;
    }

    inline void printVec2(const glm::vec2& v) { std::cout << "(" << v.x << ", " << v.y << ")" << std::endl; }

    inline void printVec3(const glm::vec3& v)
    {
        std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
    }

    inline void printVec4(const glm::vec4& v)
    {
        std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")" << std::endl;
    }

    inline void printMat3(const glm::mat3& m)
    {
        for (int i{0}; i < 3; ++i)
        {
            std::cout << "| ";
            for (int j{0}; j < 3; ++j)
            {
                std::cout << m[i][j] << "\t";
            }
            std::cout << "|" << std::endl;
        }
    }

    inline void printMat4(const glm::mat4& m)
    {
        for (int i{0}; i < 4; ++i)
        {
            std::cout << "| ";
            for (int j{0}; j < 4; ++j)
            {
                std::cout << m[i][j] << "\t";
            }
            std::cout << "|" << std::endl;
        }
    }
} // namespace Util

#endif // UTIL_H
