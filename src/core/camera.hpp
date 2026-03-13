//
// Created by Jens Kromdijk on 16/02/2025.
//

#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>

#include "engine_types.hpp"

#define CAMERA_Z_NEAR 0.1f
#define CAMERA_Z_FAR 500.0f

#define ZOOM_ENABLED

namespace CameraN
{
    enum class CameraMotion
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
    };

    // defaults
    constexpr float YAW{-90.0f};
    constexpr float PITCH{0.0f};
    constexpr float ROLL{90.0f};
    constexpr float SPEED{1.f};
    constexpr float SENSITIVITY{0.05f};
    constexpr float ZOOM{45.0f};
    // player tracking constants
    constexpr float FOLLOW_DISTANCE{20.0f};
    constexpr float Y_OFFSET{3.5f}; // look slightly above player
} // namespace CameraN

class Camera final : public EngineObject
{
public:
    explicit Camera(EngineObject* engine) : EngineObject{"Camera", engine}
    {
        m_position = glm::vec3{0.0f, 5.0f, 10.0f};
        m_front = glm::vec3{0.0f, 0.0f, -1.0f};
        m_up = glm::vec3{0.0f, 1.0f, 0.0f};
        m_worldUp = glm::vec3{0.0f, 1.0f, 0.0f};
        m_yaw = CameraN::YAW;
        m_pitch = CameraN::PITCH;
        m_roll = CameraN::ROLL;
        m_zoom = CameraN::ZOOM;
        m_movementSpeed = CameraN::SPEED;
        m_mouseSensitivity = CameraN::SENSITIVITY;

        m_followDistance = CameraN::FOLLOW_DISTANCE;
        updateCameraVectors();
    }

    [[nodiscard]] glm::mat4 getViewMatrix() const { return glm::lookAt(m_position, m_position + m_front, m_up); }

    void processInput(const CameraN::CameraMotion direction, const float deltaTime)
    {
        const float velocity{m_movementSpeed * deltaTime};
        switch (direction)
        {
        case (CameraN::CameraMotion::FORWARD):
            m_position += m_front * velocity;
            return;
        case CameraN::CameraMotion::BACKWARD:
            m_position -= m_front * velocity;
            return;
        case CameraN::CameraMotion::LEFT:
            m_position -= m_right * velocity;
            return;
        case CameraN::CameraMotion::RIGHT:
            m_position += m_right * velocity;
            break;
        }
    }

    void processMouseMovement(float xOffset, float yOffset, const GLboolean constrainPitch = true)
    {
        xOffset *= m_mouseSensitivity;
        yOffset *= m_mouseSensitivity;

        m_yaw += xOffset;
        m_pitch += yOffset;

        // cap pitch
        if (constrainPitch)
        {
            m_pitch = glm::clamp(m_pitch, -50.f, 0.f);
        }

        // update front, right & up vectors
        updateCameraVectors();
    }

    void processMouseScroll(const float yOffset)
    {
#ifdef ZOOM_ENABLED
        m_zoom -= yOffset;
        if (m_zoom < 1.0f)
        {
            m_zoom = 1.0f;
        }
        if (m_zoom > 45.0f)
        {
            m_zoom = 45.0f;
        }
#endif
    }

    // pass player body centre
    void followPlayer(const glm::vec3& position, JPH::BodyID playerID, void* jolt, float dt);

    void setZoom(const float val) { m_zoom = val; }
    [[nodiscard]] float getZoom() const { return m_zoom; }

    void setPosition(const glm::vec3& position) { m_position = position; }
    [[nodiscard]] glm::vec3 getPosition() const { return m_position; }

    void setFront(const glm::vec3& front) { m_front = front; }
    [[nodiscard]] glm::vec3 getFront() const { return m_front; }

    void setUp(const glm::vec3& up) { m_up = up; }
    [[nodiscard]] glm::vec3 getUp() const { return m_up; }

    void setRight(const glm::vec3& right) { m_right = right; }
    [[nodiscard]] glm::vec3 getRight() const { return m_right; }

    void setWorldUp(const glm::vec3& worldUp) { m_worldUp = worldUp; }
    [[nodiscard]] glm::vec3 getWorldUp() const { return m_worldUp; }

    void setYaw(const float val) { m_yaw = val; }
    [[nodiscard]] float getYaw() const { return m_yaw; }

    void setPitch(const float val) { m_pitch = val; }
    [[nodiscard]] float getPitch() const { return m_pitch; }

    void setMovementSpeed(const float val) { m_movementSpeed = val; }
    [[nodiscard]] float getMovementSpeed() const { return m_movementSpeed; }

    void setMouseSensitivity(const float val) { m_mouseSensitivity = val; }
    [[nodiscard]] float getMouseSensitivity() const { return m_mouseSensitivity; }

    void setFollowDistance(const float val) { m_followDistance = val; }
    [[nodiscard]] float getFollowDistance() const { return m_followDistance; }

    void setTargetPosition(const glm::vec3& position) { m_targetPosition = position; }
    [[nodiscard]] const glm::vec3& getTargetPosition() const { return m_targetPosition; }

    void setScreenShake(const float val) { m_screenShake = val; }
    [[nodiscard]] float getScreenShake() const { return m_screenShake; }

private:
    glm::vec3 m_position{};
    glm::vec3 m_front{};
    glm::vec3 m_up{};
    glm::vec3 m_right{};
    glm::vec3 m_worldUp{};

    float m_yaw;
    float m_pitch;
    float m_roll;
    float m_screenShake{0.0f};

    float m_movementSpeed;
    float m_mouseSensitivity;
    float m_zoom;

    float m_followDistance;
    glm::vec3 m_targetPosition{};

    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
        front.y = glm::sin(glm::radians(m_pitch));
        front.z = glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
        m_front = glm::normalize(front);

        m_right = glm::normalize(glm::cross(m_front, m_worldUp));
        m_up = glm::normalize(glm::cross(m_right, m_front));

        if (m_roll != 0.0f)
        {
            glm::mat4 rollMatrix{glm::rotate(glm::mat4{1.0f}, glm::radians(m_roll), m_front)};

            m_right = glm::vec3{rollMatrix * glm::vec4{m_right, 0.0f}};
            m_up = glm::vec3{rollMatrix * glm::vec4{m_up, 0.0f}};
        }
    }
};


#endif // CAMERA_H
