// Created by Jens Kromdijk 08/03/2026

#include "shards.hpp"
#include "core/bounds.hpp"
#include "core/physics.hpp"

ShardBody::ShardBody(Model* model, Entity* entity, JPH::BodyInterface* bodyInterface) :
    m_model{model}, m_entity{entity}, m_bodyInterface{bodyInterface}
{
}

bool ShardBody::init()
{
    const std::vector<Mesh*>& meshes1{m_model->getOpaqueMeshes()};
    const std::vector<Mesh*>& meshes2{m_model->getTransparentMeshes()};
    m_meshes.insert(m_meshes.end(), meshes1.begin(), meshes1.end());
    m_meshes.insert(m_meshes.end(), meshes2.begin(), meshes2.end());
    m_hulls.reserve(m_meshes.size());

    const Bounds::Transform& transform{m_entity->getTransform()};
    const glm::vec3 pivot{transform.getPivotOffset()};
    const glm::vec3 scale{transform.getLocalScale()};
    for (const Mesh* mesh : m_meshes)
    {
        std::vector<glm::vec3> vertices{};
        vertices.reserve(mesh->getVertices().size());
        for (const MeshN::Vertex& v : mesh->getVertices())
        {
            vertices.push_back((v.position + pivot) * scale);
        }

        m_hulls.push_back(ShapeLoader{vertices});
    }

    return true;
}

void ShardBody::explode(const float force)
{
    const Bounds::Transform& transform{m_entity->getTransform()};
    const glm::vec3 forceCenter{m_entity->getGlobalMidpoint()};
}

std::pair<Mesh*, ShapeLoader*> ShardBody::getShard(const std::size_t idx)
{
    assert(idx < m_meshes.size() && idx < m_hulls.size());
    return {m_meshes[idx], &m_hulls[idx]};
}
