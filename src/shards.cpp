// Created by Jens Kromdijk 08/03/2026

#include "shards.hpp"
#include "core/bounds.hpp"
#include "core/physics.hpp"
#include "core/engine.hpp"

#include <filesystem>

ShardBody::ShardBody(const char* name, const char* shardsFolder, Entity* entity, JPH::BodyInterface* bodyInterface) :
    m_name{name}, m_path{shardsFolder}, m_entity{entity}, m_bodyInterface{bodyInterface}
{
}

void ShardBody::init(void* engine)
{
    // load shard models
    Engine* enginePtr{static_cast<Engine*>(engine)};
    for (const auto& entry : std::filesystem::directory_iterator(m_path))
    {
        const std::filesystem::path path{entry.path()};
        const std::string extension{path.extension()};
        if (extension == ".glb" || extension == ".gltf")
        {
            ++m_numShards;
            const std::string modelName{m_name + std::to_string(m_numShards)};
            // avoid loading same model twice
            if (!enginePtr->modelExists(modelName))
            {
                enginePtr->addModel(modelName, path.string());
            }
            Model* model{enginePtr->getModel(modelName)};
            m_models.emplace_back(model);
        }
    }

    // create convex hulls
    const Bounds::Transform& transform{m_entity->getTransform()};
    const glm::vec3 pivot{transform.getPivotOffset()};
    const glm::vec3 scale{transform.getLocalScale()};

    m_hulls.reserve(m_models.size());
    for (std::size_t i{0}; i < m_models.size(); ++i)
    {
        const Model* model{m_models[i]};

        std::vector<glm::vec3> vertices{};
        for (const Mesh* mesh : model->getOpaqueMeshes())
        {
            for (const MeshN::Vertex& v : mesh->getVertices())
            {
                vertices.emplace_back((v.position + pivot) * scale);
            }
        }

        for (const Mesh* mesh : model->getTransparentMeshes())
        {
            for (const MeshN::Vertex& v : mesh->getVertices())
            {
                vertices.emplace_back((v.position + pivot) * scale);
            }
        }

        m_hulls.emplace_back(ShapeLoader{vertices});
    }
}

void ShardBody::explode(const float force)
{
    const Bounds::Transform& transform{m_entity->getTransform()};
    const glm::vec3 forceCenter{m_entity->getGlobalMidpoint()};
}

std::pair<Model*, ShapeLoader*> ShardBody::getShard(const std::size_t idx)
{
    assert(idx < m_models.size() && idx < m_hulls.size());
    return {m_models[idx], &m_hulls[idx]};
}
