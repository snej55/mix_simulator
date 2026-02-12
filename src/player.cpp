// Created by Jens Kromdijk 06-02-2026

#include "player.hpp"
#include "core/physics.hpp"

Player::Player(Entity* entity) : m_entity{entity} {}

Player::Player(const glm::vec3& pos, const Model* model)
{
    Bounds::Transform transform{pos, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};
    m_entity = new Entity{model, transform, BodyType::DYNAMIC, false};

    m_input = std::make_unique<PlayerController>(PlayerController{});
}

void Player::update(JPH::BodyInterface* bodyInterface)
{
    constexpr float speed{200000.f};
    constexpr float torque{100000.f};
    if (m_input->getControl(Controls::UP))
    {
        bodyInterface->AddForceAndTorque(m_entity->getPhysicsBody()->getBodyID(), {speed, 0.f, 0.f},
                                         {torque, 0.f, 0.f});
    }
    if (m_input->getControl(Controls::DOWN))
    {
        bodyInterface->AddForceAndTorque(m_entity->getPhysicsBody()->getBodyID(), {-speed, 0.f, 0.f},
                                         {torque, 0.f, 0.f});
    }
    if (m_input->getControl(Controls::RIGHT))
    {
        bodyInterface->AddForceAndTorque(m_entity->getPhysicsBody()->getBodyID(), {0.f, 0.f, speed},
                                         {0.f, 0.f, torque});
    }
    if (m_input->getControl(Controls::LEFT))
    {
        bodyInterface->AddForceAndTorque(m_entity->getPhysicsBody()->getBodyID(), {0.f, 0.f, -speed},
                                         {0.f, 0.f, -torque});
    }
    if (m_input->getControl(Controls::SPACE))
    {
        bodyInterface->AddForceAndTorque(m_entity->getPhysicsBody()->getBodyID(), {0.f, speed, 0.f},
                                         {0.f, torque, 0.f});
    }
}

void Player::setupPhysicsBody(JPH::BodyInterface* bodyInterface)
{
    void (*settingsModifier)(JPH::BodyCreationSettings*){[](JPH::BodyCreationSettings* settings)
                                                         {
                                                             settings->mLinearDamping = 0.5f;
                                                             settings->mAngularDamping = 0.5f;
                                                             settings->mFriction = 0.8f;
                                                         }};
    PhysicsBody physicsBody{bodyInterface,
                            *m_entity->getBoundingVolume(),
                            m_entity->getTransform().getLocalRotation(),
                            m_entity->getBodyType(),
                            m_entity->getTransform().getGlobalPosition() + m_entity->getTransform().getPivotOffset(),
                            settingsModifier};
    m_entity->setPhysicsBody(physicsBody);
    std::cout << "PLAYER::SETUP_PHYSICS_BODY: Added custom body settings!" << std::endl;
}
