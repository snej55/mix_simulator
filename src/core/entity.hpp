// Created by Jens Kromdijk 03-12-2025

#ifndef ENTITY_H
#define ENTITY_H

#include <cstddef>
#include <memory>
#include <string>

#include "model.hpp"
#include "bounds.hpp"
#include "physics.hpp"

class Entity final
{
public:
    Entity() = default;
    Entity(const Model* model, const Bounds::Transform& transform, const BodyType& bodyType, bool animated = false);
    ~Entity();

    void update(float deltaTime, const JPH::BodyInterface* bodyInterface = nullptr);

    [[nodiscard]] Bounds::AABB getGlobalAABB() const;

    [[nodiscard]] const std::string& getPath() const { return m_path; }
    [[nodiscard]] glm::vec3 getPosition() const { return m_transform.getGlobalPosition(); }

    [[nodiscard]] Model* getModel() const { return m_model.get(); }
    [[nodiscard]] bool getAnimated() const { return m_animated; }

    [[nodiscard]] const Bounds::Transform& getTransform() const { return m_transform; }
    [[nodiscard]] const Bounds::AABB* getBoundingVolume() const { return m_BV.get(); }

    void setInFrustum(const bool val) { m_inFrustum = val; }
    [[nodiscard]] bool getInFrustum() const { return m_inFrustum; }

    void setStatic(const bool val) { m_static = val; }
    [[nodiscard]] bool getStatic() const { return m_static; }

    [[nodiscard]] glm::vec3 getMidpoint() const { return m_BV->center; }
    [[nodiscard]] glm::vec3 getGlobalMidpoint() const;

    void setDiscarded(const bool val) { m_discarded = val; }
    [[nodiscard]] bool getDiscarded() const { return m_discarded; }

    void setDirty(const bool val) { m_dirty = val; }
    [[nodiscard]] bool getDirty() const { return m_dirty; }

    [[nodiscard]] const BodyType& getBodyType() const { return m_bodyType; }
    [[nodiscard]] PhysicsBody* getPhysicsBody() const { return m_physicsBody.get(); }
    void initPhysicsBody(JPH::BodyInterface* bodyInterface);

    // keep track of which chunks this entity belongs to
    void addChunk(void* chunkPtr, std::size_t index) { m_chunks.emplace_back(std::pair{index, chunkPtr}); }
    void eraseChunks() { m_chunks.clear(); }
    [[nodiscard]] const std::vector<std::pair<std::size_t, void*>>& getChunks() const { return m_chunks; }

private:
    std::string m_path{};
    Bounds::Transform m_transform{};
    std::unique_ptr<Bounds::AABB> m_BV{};

    std::unique_ptr<Model> m_model{nullptr};
    bool m_animated{false};

    bool m_inFrustum{false};
    bool m_static{false};
    BodyType m_bodyType;
    std::unique_ptr<PhysicsBody> m_physicsBody{nullptr};

    bool m_dirty{false};
    bool m_discarded{false};

    std::vector<std::pair<std::size_t, void*>> m_chunks{};
};

inline std::ostream& operator<<(std::ostream& os, const Entity& entity)
{
    os << "Entity(Model: " << entity.getPath() << ", Position: (" << entity.getPosition().x << ", "
       << entity.getPosition().y << ", " << entity.getPosition().z << "), Animated: " << std::boolalpha
       << entity.getAnimated() << ", Transform: " << entity.getTransform() << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Entity* entity)
{
    os << "Entity(Model: " << entity->getPath() << ", Position: (" << entity->getPosition().x << ", "
       << entity->getPosition().y << ", " << entity->getPosition().z << "), Animated: " << std::boolalpha
       << entity->getAnimated() << ", Transform: " << entity->getTransform() << ")";
    return os;
}

#endif // ENTITY_H
