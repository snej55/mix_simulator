// Created by Jens Kromdijk 25/02/2026

#include "camera.hpp"
#include "physics.hpp"
#include "util.hpp"

#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

void Camera::followPlayer(const glm::vec3& position, JPH::BodyID playerID, void* jolt, const float dt)
{
    if (!Util::finite3D(position) || !std::isfinite(dt))
    {
        return;
    }

    m_targetPosition += (position - m_targetPosition) * 0.1f * dt;
    const glm::vec3 focalPoint{m_targetPosition + glm::vec3(0.0f, CameraN::Y_OFFSET, 0.0f)};
    const glm::vec3 followPos{focalPoint + -m_front * m_followDistance}; // ideal position

    // raycast to check for obstacles
    JPH::RVec3 start{focalPoint.x, focalPoint.y, focalPoint.z};
    JPH::Vec3 direction{followPos.x - focalPoint.x, followPos.y - focalPoint.y, followPos.z - focalPoint.z};

    JPH::RayCast ray{start, direction};
    JPH::ClosestHitCollisionCollector<JPH::CastRayCollector> collector;

    JPH::SpecifiedObjectLayerFilter layerFilter{ObjectLayers::NON_MOVING};
    JPH::IgnoreSingleBodyFilter bodyFilter{playerID};

    JoltInstance* joltInstance{static_cast<JoltInstance*>(jolt)};

    joltInstance->getPhysicsSystem()->GetNarrowPhaseQuery().CastRay(JPH::RRayCast{ray}, JPH::RayCastSettings{},
                                                                    collector, {}, layerFilter, bodyFilter);

    if (collector.HadHit())
    {
        const glm::vec3 dir{direction.GetX(), direction.GetY(), direction.GetZ()};
        const float length{glm::length(dir)};
        const glm::vec3 dirN{(length > 1e-6f && Util::finite3D(dir)) ? (dir / length) : glm::vec3{0.0f, 0.0f, -1.0f}};

        constexpr float minFocalLength{0.35f};
        const float hitDist{length * collector.mHit.mFraction * 0.9f};
        const float focalLength{glm::clamp(hitDist, minFocalLength, m_followDistance)};

        m_position = focalPoint + dirN * focalLength;
    }
    else
    {
        m_position = followPos;
    }

    // some NaN guards
    if (Util::finite3D(focalPoint - m_position) && glm::length(focalPoint - m_position) > 1e-6)
    {
        m_front = glm::normalize(focalPoint - m_position);
    }

    const glm::vec3 right{glm::cross(m_front, m_worldUp)};
    if (Util::finite3D(right) && glm::length(right) > 1e-6)
    {
        m_right = glm::normalize(right);
    }

    const glm::vec3 up{glm::cross(m_right, m_front)};
    if (Util::finite3D(up) && glm::length(up) > 1e-6f)
    {
        m_up = glm::normalize(up);
    }

    if (m_roll != 0.0f)
    {
        glm::mat4 rollMatrix{glm::rotate(glm::mat4{1.0f}, glm::radians(m_roll), m_front)};

        m_right = glm::vec3{rollMatrix * glm::vec4{m_right, 0.0f}};
        m_up = glm::vec3{rollMatrix * glm::vec4{m_up, 0.0f}};
    }
}
