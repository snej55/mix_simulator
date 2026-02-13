// Created by Jens Kromdijk 12-02-2026


#include <limits>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/geometric.hpp>

#include "shadows.hpp"
#include "camera.hpp"
#include "engine_types.hpp"
#include "util.hpp"

Shadows::CSMGenerator::CSMGenerator(EngineObject* parent) : EngineObject{"CSMGenerator", parent} { generateSplits(); }

Shadows::CSMGenerator::~CSMGenerator() { free(); }

void Shadows::CSMGenerator::generateMaps()
{
    glGenFramebuffers(1, &m_FBO);

    glGenTextures(1, &m_textures);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_textures);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, CSM_MAP_SIZE, CSM_MAP_SIZE,
                 static_cast<GLsizei>(CASCADE_COUNT), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    constexpr float borderColor[]{1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_textures, 0, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Util::beginError();
        std::cout << "CSM_GENERATOR::GENERATE_MAPS::ERROR: Framebuffer is incomplete! Failed to generate shadow maps!";
        Util::endError();
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void Shadows::CSMGenerator::free()
{
    glDeleteFramebuffers(1, &m_FBO);
    glDeleteTextures(1, &m_textures);
    m_textures = 0;
    m_FBO = 0;
}

void Shadows::CSMGenerator::generateSplits()
{
    for (unsigned int i{1}; i < CASCADE_COUNT; ++i)
    {
        const float p{static_cast<float>(i) / static_cast<float>(CASCADE_COUNT)};
        const float log{CAMERA_Z_NEAR * std::pow(CAMERA_Z_FAR / CAMERA_Z_NEAR, p)};
        m_splits[i - 1] =
            CASCADES_LAMBDA * log + (1.0f - CASCADES_LAMBDA) * ((CAMERA_Z_FAR - CAMERA_Z_NEAR) * p + CAMERA_Z_NEAR);
    }
}

void Shadows::CSMGenerator::updateLSMatrices(const glm::mat4& view, const glm::vec3& lightDir, const float zoom,
                                             const float aspectR)
{
    std::vector<glm::vec4> corners;
    corners.reserve(8);

    for (std::size_t i{0}; i < CASCADE_COUNT; ++i)
    {
        const float near{(i == 0) ? CAMERA_Z_NEAR : m_splits[i - 1]};
        const float far{(i == CASCADE_COUNT - 1) ? CAMERA_Z_FAR : m_splits[i]};

        const glm::mat4 projection{glm::perspective(glm::radians(zoom), aspectR, near, far)};

        corners.clear();
        Util::getWorldSpaceFrustumCorners(projection, view, corners);

        glm::vec3 center{0.0f};
        for (const glm::vec4& v : corners)
        {
            center += glm::vec3(v);
        }
        center /= static_cast<float>(corners.size());

        const glm::mat4 lightView{
            glm::lookAt(center - glm::normalize(lightDir) * 100.0f, center, glm::vec3(0.0f, 1.0f, 0.0f))};

        glm::vec3 min{std::numeric_limits<float>::max()};
        glm::vec3 max{-std::numeric_limits<float>::max()};
        for (const glm::vec4& v : corners)
        {
            const glm::vec3 trf{glm::vec3(lightView * v)};
            min = glm::min(min, trf);
            max = glm::max(max, trf);
        }

        constexpr float zMult{10.0f};
        if (min.z < 0.0f)
        {
            min.z *= zMult;
        }
        else
        {
            min.z /= zMult;
        }

        if (max.z < 0.0f)
        {
            max.z /= zMult;
        }
        else
        {
            max.z *= zMult;
        }

        const glm::mat4 lightProjection{glm::ortho(min.x, max.x, min.y, max.y, min.z, max.z)};
        const glm::mat4 lightSpace{lightProjection * lightView};
        m_lightSpaceMatrices[i] = lightSpace;
    }
}
