// Created by Jens Kromdijk 06-02-2026

#include "player.hpp"
#include "core/physics.hpp"
#include "core/util.hpp"

void PlayerController::update(const float dt, Engine* engine, bool& dash)
{
    dash = false;
    if (m_dash)
    {
        m_dashTime += dt;
    }
    if (m_dashTime > 100.f && !m_dashPressed)
    {
        m_dash = false;
        m_dashTime = 0.0f;
    }

    constexpr float duration{20.f};
    if (m_dashPressed && !m_dash && m_dashTime <= duration)
    {
        m_dash = true;
        m_dashing = true;
        engine->setScreenShake(10.f);
        dash = true;
    }

    if (m_dashTime > duration)
    {
        m_dashing = false;
    }
}

Player::Player(Entity* entity) : m_entity{entity} {}

Player::Player(const glm::vec3& pos, Model* model)
{
    Bounds::Transform transform{pos, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};
    m_entity = new Entity{model, transform, BodyType::DYNAMIC, false};

    m_input = std::make_unique<PlayerController>(PlayerController{});
}

void Player::update(JPH::BodyInterface* bodyInterface, const Camera* camera)
{
    constexpr float speed{5000.f};
    constexpr float torque{1000.f};

    const glm::vec3 front{camera->getFront()};
    const glm::vec3 right{camera->getRight()};

    glm::vec3 forwardDir{0.0f, 0.0f, -1.0f};
    glm::vec3 rightDir{1.0f, 0.0f, 0.0f};
    if (Util::finite3D(front) && glm::length(front) > 1e-6f)
    {
        forwardDir = glm::normalize(front);
    }
    if (Util::finite3D(right) && glm::length(right) > 1e-6f)
    {
        rightDir = glm::normalize(right);
    }

    auto move = [&](const glm::vec3& force, const glm::vec3& torque)
    {
        bodyInterface->AddForceAndTorque(m_entity->getPhysicsBody()->getBodyID(), {force.x, force.y, force.z},
                                         {torque.x, torque.y, torque.z});
    };

    if (m_input->getControl(Controls::UP))
    {
        move({forwardDir.x * speed, 0.f, forwardDir.z * speed}, {forwardDir.x * torque, 0.f, forwardDir.z * torque});
    }
    if (m_input->getControl(Controls::DOWN))
    {
        move({-forwardDir.x * speed, 0.f, -forwardDir.z * speed},
             {-forwardDir.x * torque, 0.f, -forwardDir.z * torque});
    }
    if (m_input->getControl(Controls::RIGHT))
    {
        move({rightDir.x * speed, 0.f, rightDir.z * speed}, {rightDir.x * torque, 0.f, -rightDir.z * torque});
    }
    if (m_input->getControl(Controls::LEFT))
    {
        move({-rightDir.x * speed, 0.f, -rightDir.z * speed}, {-rightDir.x * torque, 0.f, rightDir.z * torque});
    }
    if (m_input->getControl(Controls::SPACE) && m_jump)
    {
        move({0.f, speed, 0.f}, {0.f, torque, 0.f});
    }

    if (m_input->getDashing())
    {
        constexpr float dashSpeed{50000.f};
        move({forwardDir.x * dashSpeed, 0.f, forwardDir.z * dashSpeed},
             {forwardDir.x * torque, 0.f, forwardDir.z * torque});
    }

    float y{m_entity->getGlobalMidpoint().y};
    if (y < 0.1f)
    {
        m_jump = true;
    }
    if (y > 2.f)
    {
        m_jump = false;
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

    // create convex hull
    std::vector<glm::vec3> vertices{};
    const glm::vec3 pivot{m_entity->getTransform().getPivotOffset()};
    const glm::vec3 scale{m_entity->getTransform().getLocalScale()};
    for (const Mesh* mesh : m_entity->getModel()->getOpaqueMeshes())
    {
        for (const MeshN::Vertex& v : mesh->getVertices())
        {
            vertices.push_back((v.position + pivot) * scale);
        }
    }
    for (const Mesh* mesh : m_entity->getModel()->getTransparentMeshes())
    {
        for (const MeshN::Vertex& v : mesh->getVertices())
        {
            vertices.push_back((v.position + pivot) * scale);
        }
    }
    PhysicsBody physicsBody{bodyInterface,
                            vertices,
                            m_entity->getTransform().getLocalRotation(),
                            m_entity->getBodyType(),
                            m_entity->getTransform().getGlobalPosition() + m_entity->getTransform().getPivotOffset(),
                            settingsModifier,
                            0.2f};
    m_entity->setPhysicsBody(physicsBody);
    std::cout << "PLAYER::SETUP_PHYSICS_BODY: Added custom body settings!" << std::endl;
}
