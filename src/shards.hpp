// Created by Jens Kromdijk 08/03/2026

#ifndef SHARDS_H
#define SHARDS_H

#include "core/model.hpp"
#include "core/entity.hpp"
#include "core/physics.hpp"

#include <string>

class ShardBody
{
public:
    ShardBody(const char* name, const char* shardsFolder, Entity* entity, JPH::BodyInterface* bodyInterface);
    ~ShardBody() = default;

    void init(void* engine);

    [[nodiscard]] std::string_view getName() const { return m_name; }
    [[nodiscard]] std::string_view getPath() const { return m_path; }
    [[nodiscard]] Entity* getEntity() const { return m_entity; }
    [[nodiscard]] const std::vector<Model*>& getMeshes() const { return m_models; }
    [[nodiscard]] const std::vector<ShapeLoader>& getPhysicsBodies() const { return m_hulls; }
    [[nodiscard]] std::size_t getNumShards() const { return m_numShards; }

    [[nodiscard]] std::pair<Model*, ShapeLoader*> getShard(std::size_t idx);

private:
    std::string m_name;
    std::string m_path;
    Entity* m_entity;
    JPH::BodyInterface* m_bodyInterface;

    std::size_t m_numShards{0};
    std::vector<Model*> m_models{}; // just to combine transparent and opaque meshes
    std::vector<ShapeLoader> m_hulls{};
};

#endif
