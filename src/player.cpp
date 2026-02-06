// Created by Jens Kromdijk 06-02-2026

#include "player.hpp"

Player::Player(Entity* entity) : m_entity{entity} {}

Player::Player(const glm::vec3& pos, const Model* model)
{
    Bounds::Transform transform{pos, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};
    m_entity = new Entity{model, transform, BodyType::DYNAMIC, false};
}
