// Created by Jens Kromdijk 12-02-2026

#ifndef SHADOWS_H
#define SHADOWS_H

#include <glm/glm.hpp>

#include <array>

namespace Shadows
{
    inline constexpr std::size_t CASCADE_COUNT{4};
    inline constexpr float CASCADES_LAMBDA{0.5f};
    inline constexpr float CASCADE_RADIUS{100.f};
    inline constexpr int CSM_MAP_SIZE{2048};

    class CSMGenerator
    {
    public:
        CSMGenerator();
        ~CSMGenerator();

        void generateMaps();
        void free();

        [[nodiscard]] const std::array<float, CASCADE_COUNT>& getSplits() const { return m_splits; }
        [[nodiscard]] const std::array<glm::mat4, CASCADE_COUNT>& getLightSpaceMatrices() const
        {
            return m_lightSpaceMatrices;
        }
        void updateLSMatrices(const glm::mat4& view, const glm::vec3& lightDir, float zoom, float aspectR);

        [[nodiscard]] unsigned int getTextures() const { return m_textures; }
        [[nodiscard]] unsigned int getFBO() const { return m_FBO; }

    private:
        std::array<float, CASCADE_COUNT> m_splits{};
        std::array<glm::mat4, CASCADE_COUNT> m_lightSpaceMatrices{};

        unsigned int m_textures{0}; // GL_TEXTURE_2D_ARRAY
        unsigned int m_FBO{0};

        void generateSplits();
    };
} // namespace Shadows


#endif
