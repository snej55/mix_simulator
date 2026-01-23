// Created by Jens Kromdijk 18-01-2026

#ifndef LIGHTS_H
#define LIGHTS_H

#include <glm/glm.hpp>
#include <iostream>

#include "bounds.hpp"

namespace Lights
{
    inline constexpr float POINT_LIGHT_RENDER_SCALE{0.1f};
    inline constexpr std::size_t MAX_POINT_LIGHTS{32};

    struct PointLight
    {
        glm::vec3 m_position{0.0f, 0.0f, 0.0f};
        glm::vec3 m_color{1.0f, 1.0f, 1.0f};
        float m_radius{1.0f};

        PointLight(const glm::vec3& position, const glm::vec3& color, float radius);

        // prob shoudn't be member functions but we need a light to check with right?
        bool intersectsLight(const PointLight& light) const;
        bool intersectsAABB(const Bounds::AABB& aabb) const;

        void modifyModelMesh(Mesh* mesh);
    };

    inline std::ostream& operator<<(std::ostream& os, const PointLight& pLight)
    {
        os << "PointLight{position: (" << pLight.m_position.x << ", " << pLight.m_position.y << ", "
           << pLight.m_position.z << "), color: (" << pLight.m_color.x << ", " << pLight.m_color.y << ", "
           << pLight.m_color.z << "), radius: " << pLight.m_radius << "}";
        return os;
    }
} // namespace Lights

#endif
