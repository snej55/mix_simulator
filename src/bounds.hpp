//
// Created by Jens Kromdijk on 06/12/2025.
//

#ifndef MAIN_BOUNDS_HPP
#define MAIN_BOUNDS_HPP

#include <glm/glm.hpp>

#include "camera.hpp"

namespace Bounds
{
    class Transform
    {
    public:
        void computeModelMatrix();
        void computeModelMatrix(const glm::mat4& parentGlobalModelMatrix);
        void setLocalPosition(const glm::vec3& localPosition);
        [[nodiscard]] const glm::vec3& localPosition() const {return m_pos;};
        [[nodiscard]] const glm::mat4& getModelMat() const {return m_modelMat;}
        [[nodiscard]] bool getDirty() const {return m_dirty;}

    protected:
        glm::vec3 m_pos {0.0f, 0.0f, 0.0f};
        glm::vec3 m_eulerRot {0.0f, 0.0f, 0.0f};
        glm::vec3 m_scale {1.0f, 1.0f, 1.0f};

        glm::mat4 m_modelMat {1.0f};

        bool m_dirty{true};

        glm::mat4 getLocalModelMat() const;
    };

    struct Plane
    {
        glm::vec3 normal {0.f, 1.f, 0.f};
        float distance {0.f};

        Plane() = default;
        Plane(const glm::vec3& p1, const glm::vec3& norm);
        [[nodiscard]] float getSignedDistance(const glm::vec3& p) const;
    };

    struct Frustum
    {
        Plane topFace;
        Plane bottomFace;
        Plane rightFace;
        Plane leftFace;
        Plane farFace;
        Plane nearFace;
    };

    Frustum createFrustum(const Camera& cam, float aspectR, float fovY, float zNear, float zFar);
}

#endif //MAIN_BOUNDS_HPP