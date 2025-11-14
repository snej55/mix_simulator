//
// Created by Jens Kromdijk on 16/02/2025.
//

#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "engine_types.hpp"

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
    constexpr float SPEED{15.f};
    constexpr float SENSITIVITY{0.05f};
    constexpr float ZOOM{45.0f};
} // namespace CameraN

class Camera final : public EngineObject
{
public:
    explicit Camera(EngineObject* engine) : EngineObject{"Camera", engine}
    {
        m_position = glm::vec3(0.0f, 0.0f, 3.0f);
        m_front = glm::vec3(0.0f, 0.0f, -1.0f);
        m_up = glm::vec3(0.0f, 1.0f, 0.0f);
        m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        m_yaw = CameraN::YAW;
        m_pitch = CameraN::PITCH;
        m_zoom = CameraN::ZOOM;
        m_movementSpeed = CameraN::SPEED;
        m_mouseSensitivity = CameraN::SENSITIVITY;
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
            if (m_pitch > 89.0f)
            {
                m_pitch = 89.0f;
            }
            if (m_pitch < -89.0f)
            {
                m_pitch = -89.0f;
            }
        }

        // update front, right & up vectors
        updateCameraVectors();
    }

    void processMouseScroll(const float yOffset)
    {
        m_zoom -= yOffset;
        if (m_zoom < 1.0f)
        {
            m_zoom = 1.0f;
        }
        if (m_zoom > 45.0f)
        {
            m_zoom = 45.0f;
        }
    }

    [[nodiscard]] float getZoom() const { return m_zoom; }

    [[nodiscard]] glm::vec3 getPosition() const { return m_position; }

    [[nodiscard]] glm::vec3 getFront() const { return m_front; }

    [[nodiscard]] glm::vec3 getUp() const { return m_up; }

    [[nodiscard]] glm::vec3 getRight() const { return m_right; }

    [[nodiscard]] glm::vec3 getWorldUp() const { return m_worldUp; }

    [[nodiscard]] float getYaw() const { return m_yaw; }

    [[nodiscard]] float getPitch() const { return m_pitch; }

    [[nodiscard]] float getMovementSpeed() const { return m_movementSpeed; }

    [[nodiscard]] float getMouseSensitivity() const { return m_mouseSensitivity; }

private:
    glm::vec3 m_position{};
    glm::vec3 m_front{};
    glm::vec3 m_up{};
    glm::vec3 m_right{};
    glm::vec3 m_worldUp{};

    float m_yaw;
    float m_pitch;

    float m_movementSpeed;
    float m_mouseSensitivity;
    float m_zoom;

    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
        front.y = glm::sin(glm::radians(m_pitch));
        front.z = glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
        m_front = glm::normalize(front);
        m_right = glm::normalize(glm::cross(m_front, m_worldUp));
        m_up = glm::normalize(glm::cross(m_right, m_front));
    }
};


#endif // CAMERA_H
