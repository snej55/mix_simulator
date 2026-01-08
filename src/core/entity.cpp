// Created by Jens Kromdijk 03-12-2025

#include <memory>

#include "entity.hpp"
#include "bounds.hpp"
#include "physics.hpp"

Entity::Entity(const Model* model, const Bounds::Transform& transform, const BodyType& bodyType, const bool animated) :
    m_transform{transform}, m_bodyType{bodyType}, m_animated{animated}
{
    m_transform.computeModelMatrix();
    m_model = std::make_unique<Model>(*model);
    m_path = m_model->getPath();
    m_BV = std::make_unique<Bounds::AABB>(Bounds::generateAABB_BV(m_model.get(), m_transform.getLocalScale()));
    m_static = (bodyType == BodyType::STATIC);
}

Entity::~Entity() = default;

void Entity::update(const float deltaTime, const JPH::BodyInterface* bodyInterface)
{
    if (bodyInterface != nullptr && m_physicsBody.get() != nullptr)
    {
        m_physicsBody->syncTransform(m_transform, bodyInterface);
    }
    if (m_transform.getDirty())
    {
        // TODO: Integrate with scene graph?
        m_transform.computeModelMatrix();
    }

    if (m_animated)
    {
        m_model->updateAnimation(deltaTime);
    }
}

Bounds::AABB Entity::getGlobalAABB() const
{
    const glm::vec3 globalCenter{m_transform.getModelMat() * glm::vec4{m_BV->center, 1.f}};

    // scaled orientation
    const glm::vec3 right{m_transform.getRight() * m_BV->extents.x};
    const glm::vec3 up{m_transform.getUp() * m_BV->extents.y};
    const glm::vec3 forward{m_transform.getForward() * m_BV->extents.z};

    // new x extent
    const float nEX{std::abs(glm::dot(glm::vec3{1.0f, 0.0f, 0.0f}, right)) +
                    std::abs(glm::dot(glm::vec3{1.0f, 0.0f, 0.0f}, up)) +
                    std::abs(glm::dot(glm::vec3{1.0f, 0.0f, 0.0f}, forward))};
    // new y extent
    const float nEY{std::abs(glm::dot(glm::vec3{0.0f, 1.0f, 0.0f}, right)) +
                    std::abs(glm::dot(glm::vec3{0.0f, 1.0f, 0.0f}, up)) +
                    std::abs(glm::dot(glm::vec3{0.0f, 1.0f, 0.0f}, forward))};
    // new z extent
    const float nEZ{std::abs(glm::dot(glm::vec3{0.0f, 0.0f, 1.0f}, right)) +
                    std::abs(glm::dot(glm::vec3{0.0f, 0.0f, 1.0f}, up)) +
                    std::abs(glm::dot(glm::vec3{0.0f, 0.0f, 1.0f}, forward))};

    return Bounds::AABB{globalCenter, nEX, nEY, nEZ};
}

glm::vec3 Entity::getGlobalMidpoint() const
{
    const glm::vec3 globalCenter{m_transform.getModelMat() * glm::vec4{m_BV->center, 1.f}};
    return glm::vec3(globalCenter);
}

void Entity::initPhysicsBody(JPH::BodyInterface* bodyInterface)
{
    assert(m_physicsBody.get() == nullptr);
    m_physicsBody =
        std::make_unique<PhysicsBody>(bodyInterface, getGlobalAABB(), m_transform.getLocalRotation(), m_bodyType);
}
