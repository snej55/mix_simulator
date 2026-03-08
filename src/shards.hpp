// Created by Jens Kromdijk 08/03/2026

#ifndef SHARDS_H
#define SHARDS_H

#include "core/model.hpp"
#include "core/entity.hpp"
#include "core/physics.hpp"

class ShardBody
{
public:
    ShardBody(Model* model, Entity* entity);
    ShardBody(Model* model, Entity* entity, JPH::BodyInterface* bodyInterface);
    ~ShardBody() = default;

    bool init();
    void explode(float force);

    [[nodiscard]] Model* getModel() const { return m_model; }
    [[nodiscard]] Entity* getEntity() const { return m_entity; }
    [[nodiscard]] const std::vector<Mesh*>& getMeshes() const { return m_meshes; }
    [[nodiscard]] const std::vector<ShapeLoader>& getPhysicsBodies() const { return m_hulls; }

    [[nodiscard]] std::pair<Mesh*, ShapeLoader*> getShard(std::size_t idx);

private:
    Model* m_model;
    Entity* m_entity;
    JPH::BodyInterface* m_bodyInterface;

    std::vector<Mesh*> m_meshes{}; // just to combine transparent and opaque meshes
    std::vector<ShapeLoader> m_hulls{};
};

#endif
