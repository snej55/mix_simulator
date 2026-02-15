// Created by Jens Kromdijk 12-02-2026

#ifndef SHADOWS_H
#define SHADOWS_H

#include "engine_types.hpp"
#include "camera.hpp"

#include <array>
#include <vector>

namespace Shadows
{
    inline constexpr std::array<float, 4> shadowCascadeLevels{CAMERA_Z_FAR / 50.0f, CAMERA_Z_FAR / 25.0f,
                                                              CAMERA_Z_FAR / 10.0f, CAMERA_Z_FAR / 2.0f};
    inline constexpr unsigned int shadowMapSize{4096};
}; // namespace Shadows

class CSMGenerator final : public EngineObject
{
public:
    explicit CSMGenerator(EngineObject* parent);
    ~CSMGenerator();

    void init();
    void free();

    [[nodiscard]] std::vector<glm::mat4> getLightSpaceMatrices(const float zoom, const float aspectR,
                                                               const glm::mat4& view, const glm::vec3& lightDir) const;

    [[nodiscard]] unsigned int getFBO() const { return m_FBO; }
    [[nodiscard]] unsigned int getDepthMaps() const { return m_depthMaps; }

private:
    unsigned int m_FBO{0};
    unsigned int m_depthMaps{0};

    glm::mat4 getLSMatrix(const float zoom, const float aspectR, const float near, const float far,
                          const glm::mat4& view, const glm::vec3& lightDir) const;
};

#endif
