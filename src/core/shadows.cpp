// Created by Jens Kromdijk 12-02-2026

#include "shadows.hpp"
#include <limits>
#include "camera.hpp"
#include "engine_types.hpp"
#include "util.hpp"

CSMGenerator::CSMGenerator(EngineObject* parent) : EngineObject{"CSMGenerator", parent} { init(); }

CSMGenerator::~CSMGenerator() { free(); }

void CSMGenerator::init()
{
    glGenFramebuffers(1, &m_FBO);

    glGenTextures(1, &m_depthMaps);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_depthMaps);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, Shadows::shadowMapSize, Shadows::shadowMapSize,
                 static_cast<int>(Shadows::shadowCascadeLevels.size()) + 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    constexpr float borderColor[]{1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthMaps, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Util::beginError();
        std::cout << "CSM_GENERATOR::INIT::ERROR: Framebuffer is incomplete! Failed to initialize CSM Generator!"
                  << std::endl;
        Util::endError();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CSMGenerator::free()
{
    glDeleteFramebuffers(1, &m_FBO);
    glDeleteTextures(1, &m_depthMaps);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

std::vector<glm::mat4> CSMGenerator::getLightSpaceMatrices(const float zoom, const float aspectR, const glm::mat4& view,
                                                           const glm::vec3& lightDir) const
{
    std::vector<glm::mat4> ret;
    for (std::size_t i{0}; i < Shadows::shadowCascadeLevels.size() + 1; ++i)
    {
        if (i == 0)
        {
            ret.push_back(getLSMatrix(zoom, aspectR, CAMERA_Z_NEAR, Shadows::shadowCascadeLevels[i], view, lightDir));
        }
        else if (i < Shadows::shadowCascadeLevels.size())
        {
            ret.push_back(getLSMatrix(zoom, aspectR, Shadows::shadowCascadeLevels[i - 1],
                                      Shadows::shadowCascadeLevels[i], view, lightDir));
        }
        else
        {
            ret.push_back(
                getLSMatrix(zoom, aspectR, Shadows::shadowCascadeLevels[i - 1], CAMERA_Z_FAR, view, lightDir));
        }
    }
    return ret;
}

glm::mat4 CSMGenerator::getLSMatrix(const float zoom, const float aspectR, const float near, const float far,
                                    const glm::mat4& view, const glm::vec3& lightDir) const
{
    const glm::mat4 projection{glm::perspective(glm::radians(zoom), aspectR, near, far)};
    const std::vector<glm::vec4> corners{Util::getFrustumCornersWorldSpace(projection, view)};

    glm::vec3 center{0.0f, 0.0f, 0.0f};
    for (const glm::vec4& v : corners)
    {
        center += glm::vec3(v);
    }
    center /= static_cast<float>(corners.size());

    const glm::mat4 lightView{glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f))};

    glm::vec3 min{std::numeric_limits<float>::max()};
    glm::vec3 max{std::numeric_limits<float>::lowest()};

    for (const glm::vec4& v : corners)
    {
        const auto trf{lightView * v};
        min.x = std::min(min.x, trf.x);
        min.y = std::min(min.y, trf.y);
        min.z = std::min(min.z, trf.z);
        max.x = std::max(max.x, trf.x);
        max.y = std::max(max.y, trf.y);
        max.z = std::max(max.z, trf.z);
    }

    constexpr float zMult{5.0f};
    if (min.z < 0.0)
    {
        min.z *= zMult;
    }
    else
    {
        min.z /= zMult;
    }
    if (max.z < 0)
    {
        max.z /= zMult;
    }
    else
    {
        max.z *= zMult;
    }

    float texelSize{(max.x - min.x) / static_cast<float>(Shadows::shadowMapSize)};
    min.x = std::floor(min.x / texelSize) * texelSize;
    max.x = std::floor(max.x / texelSize) * texelSize;
    texelSize = (max.y - min.y) / static_cast<float>(Shadows::shadowMapSize);
    min.y = std::floor(min.y / texelSize) * texelSize;
    max.y = std::floor(max.y / texelSize) * texelSize;
    texelSize = (max.z - min.z) / static_cast<float>(Shadows::shadowMapSize);
    min.z = std::floor(min.z / texelSize) * texelSize;
    max.z = std::floor(max.z / texelSize) * texelSize;

    const glm::mat4 lightProjection{glm::ortho(min.x, max.x, min.y, max.y, min.z, max.z)};
    return lightProjection * lightView;
}
