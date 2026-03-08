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
    m_transform.setPivotOffset(-m_BV->center);
    m_transform.computeModelMatrix();
    m_static = (bodyType == BodyType::STATIC);
}

Entity::~Entity() { setEntityController(nullptr); }

void Entity::update(const float deltaTime, const JPH::BodyInterface* bodyInterface)
{
    if (m_callback != nullptr)
    {
        m_callback(this);
    }
    if (m_controller != nullptr)
    {
        m_controller->update();
    }

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

void Entity::initPhysicsBody(JPH::BodyInterface* bodyInterface, const bool simple)
{
    if (m_physicsBody.get() != nullptr)
    {
        std::cout << "ENTITY::INIT_PHYSICS_BODY::ERROR: Physics body is already initialized!" << std::endl;
        return;
    }

    Bounds::AABB localAABB{*m_BV};
    if (simple)
    {
        m_physicsBody =
            std::make_unique<PhysicsBody>(bodyInterface, localAABB, m_transform.getLocalRotation(), m_bodyType,
                                          m_transform.getGlobalPosition() + m_transform.getPivotOffset());
    }
    else
    {
        std::vector<glm::vec3> vertices{};
        const glm::vec3 pivot{m_transform.getPivotOffset()};
        const glm::vec3 scale{m_transform.getLocalScale()};
        for (const Mesh* mesh : m_model->getOpaqueMeshes())
        {
            for (const MeshN::Vertex& v : mesh->getVertices())
            {
                vertices.push_back((v.position + pivot) * scale);
            }
        }
        for (const Mesh* mesh : m_model->getTransparentMeshes())
        {
            for (const MeshN::Vertex& v : mesh->getVertices())
            {
                vertices.push_back((v.position + pivot) * scale);
            }
        }

        // do a convex hull
        m_physicsBody =
            std::make_unique<PhysicsBody>(bodyInterface, vertices, m_transform.getLocalRotation(), m_bodyType,
                                          m_transform.getGlobalPosition() + m_transform.getPivotOffset());
    }
}

void Entity::setPhysicsBody(PhysicsBody& physicsBody) { m_physicsBody = std::make_unique<PhysicsBody>(physicsBody); }

void Entity::setEntityController(EntityController* controller)
{
    if (m_controller != nullptr)
    {
        delete m_controller;
        m_controller = nullptr;
    }

    m_controller = controller;
}
