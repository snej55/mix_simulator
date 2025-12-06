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
        void setLocalRotation(const glm::vec3& localRotation);
        void setLocalScale(const glm::vec3& localScale);

        [[nodiscard]] glm::vec3 getGlobalPosition() const { return m_modelMat[3]; }
        [[nodiscard]] glm::vec3 getRight() const { return m_modelMat[0]; }
        [[nodiscard]] glm::vec3 getUp() const { return m_modelMat[1]; }
        [[nodiscard]] glm::vec3 getBackward() const { return m_modelMat[2]; }
        [[nodiscard]] glm::vec3 getForward() const { return -m_modelMat[2]; }
        [[nodiscard]] glm::vec3 getGlobalScale() const
        {
            return {glm::length(getRight()), glm::length(getUp()), glm::length(getBackward())};
        }

        [[nodiscard]] const glm::vec3& localPosition() const { return m_pos; };
        [[nodiscard]] const glm::mat4& getModelMat() const { return m_modelMat; }
        [[nodiscard]] const glm::vec3& getLocalRotation() const { return m_eulerRot; }
        [[nodiscard]] const glm::vec3& getLocalScale() const { return m_scale; }
        [[nodiscard]] bool getDirty() const { return m_dirty; }

    protected:
        glm::vec3 m_pos{0.0f, 0.0f, 0.0f};
        glm::vec3 m_eulerRot{0.0f, 0.0f, 0.0f};
        glm::vec3 m_scale{1.0f, 1.0f, 1.0f};

        glm::mat4 m_modelMat{1.0f};

        bool m_dirty{true};

        glm::mat4 getLocalModelMat() const;
    };

    struct Plane
    {
        glm::vec3 normal{0.f, 1.f, 0.f};
        float distance{0.f};

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

    inline Frustum createFrustum(const Camera& cam, float aspectR, float fovY, float zNear, float zFar);

    // bounding volume for frustum culling
    struct Volume
    {
        Volume() = default;
        virtual bool onFrustum(const Frustum& camFrustum, const Transform& modelTransform) const = 0;
        virtual bool onForwardPlane(const Plane& plane) const = 0;

        bool onFrustum(const Frustum& camFrustum) const;
    };

    struct Sphere final : public Volume
    {
        glm::vec3 center{0.f, 0.f, 0.f};
        float radius{0.f};

        Sphere() = default;
        Sphere(const glm::vec3& aCenter, float aRadius);

        bool onForwardPlane(const Plane& plane) const override;
        bool onFrustum(const Frustum& camFrustum, const Transform& modelTransform) const override;
    };
} // namespace Bounds

#endif // MAIN_BOUNDS_HPP
