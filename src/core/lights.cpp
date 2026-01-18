// Created by Jens Kromdijk 18-01-2026

#include "lights.hpp"

Lights::PointLight::PointLight(const glm::vec3& position, const glm::vec3& color, const float radius) :
    m_position{position}, m_color{color}, m_radius{radius}
{
}

bool Lights::PointLight::intersectsLight(const PointLight& pLight) const
{
    return glm::length(pLight.m_position - m_position) < pLight.m_radius + m_radius;
}

bool Lights::PointLight::intersectsAABB(const Bounds::AABB& aabb) const
{
    const glm::vec3 min{aabb.center - aabb.extents};
    const glm::vec3 max{aabb.center + aabb.extents};

    const glm::vec3 closestPoint{glm::clamp(m_position, min, max)};
    const glm::vec3 delta{m_position - closestPoint};
    return glm::dot(delta, delta) < m_radius * m_radius;
}
