// Created by Jens Kromdijk 06-02-2025

#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>

#include "core/entity.hpp"
#include "core/iohandler.hpp"
#include "core/engine.hpp"

enum class Controls
{
    UP = 0x0,
    DOWN = 0x1,
    RIGHT = 0x2,
    LEFT = 0x3,
    SPACE = 0x4,
    NONE = 0x5
};

class PlayerController : public Controller<Controls>
{
public:
    PlayerController() : Controller<Controls>{} {};

    void update(float dt, Engine* engine);

    void setDashedPressed(const bool val) { m_dashPressed = val; }
    [[nodiscard]] bool getDashedPressed() const { return m_dashPressed; }
    [[nodiscard]] bool getDash() const { return m_dash; }
    [[nodiscard]] float getDashTime() const { return m_dashTime; }
    [[nodiscard]] bool getDashing() const { return m_dashing; }

private:
    bool m_dashPressed{false};
    float m_dashTime{0.0f};
    bool m_dash{false};
    bool m_dashing{false};
};

class Player
{
public:
    Player() = default;
    explicit Player(Entity* entity);
    Player(const glm::vec3& pos, Model* model);

    ~Player() = default;

    void update(JPH::BodyInterface* bodyInterface, const Camera* camera);

    void setupPhysicsBody(JPH::BodyInterface* bodyInterface);

    [[nodiscard]] Entity* getEntity() const { return m_entity; }
    void setEntity(Entity* entity) { m_entity = entity; }
    [[nodiscard]] PlayerController* getController() { return m_input.get(); }

    [[nodiscard]] glm::vec2 get2DPos() const
    {
        return {m_entity->getGlobalMidpoint().x, m_entity->getGlobalMidpoint().z};
    }

private:
    Entity* m_entity{nullptr};
    bool m_jump{false};

    std::unique_ptr<PlayerController> m_input{nullptr};
};

#endif
