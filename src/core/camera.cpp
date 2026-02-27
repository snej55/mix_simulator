// Created by Jens Kromdijk 25/02/2026

#include "camera.hpp"
#include "physics.hpp"

#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

void Camera::followPlayer(const glm::vec3& position, JPH::BodyID playerID, void* jolt, const float dt)
{
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
        m_position = focalPoint +
            (glm::vec3{direction.GetX(), direction.GetY(), direction.GetZ()} * collector.mHit.mFraction * 0.9f);
    }
    else
    {
        m_position = followPos;
    }

    m_front = glm::normalize(focalPoint - m_position);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}
