// Created by Jens Kromdijk 06-02-2025

#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>

#include "core/entity.hpp"

class Player
{
public:
    Player() = default;
    explicit Player(Entity* entity);
    Player(const glm::vec3& pos, const Model* model);

    ~Player() = default;

    void update();

    [[nodiscard]] Entity* getEntity() const { return m_entity; }
    void setEntity(Entity* entity) { m_entity = entity; }

private:
    Entity* m_entity{nullptr};
};

#endif
