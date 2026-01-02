//
// Created by Jens Kromdijk on 06/12/2025.
//

#ifndef MAIN_BOUNDS_HPP
#define MAIN_BOUNDS_HPP

#include <glm/glm.hpp>
#include <ostream>

#include "camera.hpp"
#include "model.hpp"

namespace Bounds
{
    class Transform
    {
    public:
        Transform() = default;
        Transform(const glm::vec3& pos, const glm::vec3& rotation, const glm::vec3& scale);

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

        [[nodiscard]] glm::mat4 getLocalModelMat() const;
    };

    inline std::ostream& operator<<(std::ostream& os, const Transform& transform)
    {
        os << std::boolalpha << "Transform(Position: (" << transform.getGlobalPosition().x << ", "
           << transform.getGlobalPosition().y << ", " << transform.getGlobalPosition().z << "), Rotation: ("
           << transform.getLocalRotation().x << ", " << transform.getLocalRotation().y << ", "
           << transform.getLocalRotation().z << "), Scale: (" << transform.getLocalScale().x << ", "
           << transform.getLocalScale().y << ", " << transform.getLocalScale().z << "), Dirty: " << transform.getDirty()
           << ")";
        return os;
    }

    struct Plane
    {
        glm::vec3 normal{0.f, 1.f, 0.f};
        float distance{0.f};
        glm::vec3 p1{0.f, 0.f, 0.f};

        Plane() = default;
        Plane(const glm::vec3& aP1, const glm::vec3& norm);
        [[nodiscard]] float getSignedDistance(const glm::vec3& p) const;
    };

    inline std::ostream& operator<<(std::ostream& os, const Plane& plane)
    {
        os << "Plane(Point: (" << plane.p1.x << ", " << plane.p1.y << ", " << plane.p1.z << "), Normal: ("
           << plane.normal.x << ", " << plane.normal.y << ", " << plane.normal.z << "), Distance: " << plane.distance
           << ")";
        return os;
    }

    struct Frustum
    {
        Plane topFace;
        Plane bottomFace;
        Plane rightFace;
        Plane leftFace;
        Plane farFace;
        Plane nearFace;
    };

    inline std::ostream& operator<<(std::ostream& os, const Frustum& frustum)
    {
        os << "Frustum(\n"
           << "  Top Face: " << frustum.topFace << "\n"
           << "  Bottom Face: " << frustum.bottomFace << "\n"
           << "  Right Face: " << frustum.rightFace << "\n"
           << "  Left Face: " << frustum.leftFace << "\n"
           << "  Far Face: " << frustum.farFace << "\n"
           << "  Near Face: " << frustum.nearFace << "\n"
           << ")";
        return os;
    }

    Frustum createFrustum(const Camera* cam, float aspectR, float fovY, float zNear, float zFar);

    // bounding volume for frustum culling
    struct Volume
    {
        Volume() = default;
        virtual ~Volume() = default;
        [[nodiscard]] virtual bool onFrustum(const Frustum& camFrustum, const Transform& modelTransform) const = 0;
        [[nodiscard]] virtual bool onForwardPlane(const Plane& plane) const = 0;
        [[nodiscard]] virtual bool collidePoint(const glm::vec3& point) const = 0;

        [[nodiscard]] bool onFrustum(const Frustum& camFrustum) const;
    };

    // simple sphere bounding volume
    struct Sphere final : Volume
    {
        glm::vec3 center{0.f, 0.f, 0.f};
        float radius{0.f};

        Sphere() = default;
        Sphere(const glm::vec3& aCenter, float aRadius);

        [[nodiscard]] bool onForwardPlane(const Plane& plane) const override;
        [[nodiscard]] bool onFrustum(const Frustum& camFrustum, const Transform& modelTransform) const override;
        [[nodiscard]] bool collidePoint(const glm::vec3& point) const override;
        [[nodiscard]] bool collideSphere(const Sphere& other) const;
    };

    inline std::ostream& operator<<(std::ostream& os, const Sphere& sphere)
    {
        os << "Sphere(Center: (" << sphere.center.x << ", " << sphere.center.y << ", " << sphere.center.z
           << "), Radius: " << sphere.radius << ")";
        return os;
    }

    Sphere generateSphereBV(const Model* model);

    // AABB bounding volume
    struct AABB final : Volume
    {
        glm::vec3 center{0.f, 0.f, 0.f};
        glm::vec3 extents{0.f, 0.f, 0.f};

        AABB() = default;
        AABB(const glm::vec3& min, const glm::vec3& max); // construct between two points
        AABB(const glm::vec3& aCenter, float eX, float eY, float eZ); // construct using center and extent

        [[nodiscard]] bool onForwardPlane(const Plane& plane) const override;
        [[nodiscard]] bool onFrustum(const Frustum& camFrustum, const Transform& modelTransform) const override;
        [[nodiscard]] bool onFrustum(const Frustum& camFrustum, const Transform& modelTransform, float padding) const;
        [[nodiscard]] bool collidePoint(const glm::vec3& point) const override;
        [[nodiscard]] bool collideAABB(const AABB& other) const;
    };

    inline std::ostream& operator<<(std::ostream& os, const AABB& aabb)
    {
        os << "AABB(Center: (" << aabb.center.x << ", " << aabb.center.y << ", " << aabb.center.z << "), Extents: ("
           << aabb.extents.x << ", " << aabb.extents.y << ", " << aabb.extents.z << "))";
        return os;
    }

    AABB generateAABB_BV(const Model* model);
    AABB getFrustumBV(const Frustum& frustum, const Camera* cam, float zFar, float fovY, float aspectR);
} // namespace Bounds

#endif // MAIN_BOUNDS_HPP
