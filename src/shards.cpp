// Created by Jens Kromdijk 08/03/2026

#include "shards.hpp"
#include "core/bounds.hpp"
#include "core/physics.hpp"
#include "core/engine.hpp"
#include "core/util.hpp"

#include <algorithm>
#include <filesystem>
#include <limits>
#include <glm/gtx/quaternion.hpp>

namespace
{
    glm::vec3 getModelLocalCenter(const Model* model)
    {
        glm::vec3 minV{std::numeric_limits<float>::max()};
        glm::vec3 maxV{std::numeric_limits<float>::lowest()};

        auto accumulate = [&](const std::vector<Mesh*>& meshes)
        {
            for (const Mesh* mesh : meshes)
            {
                for (const MeshN::Vertex& v : mesh->getVertices())
                {
                    minV = glm::min(minV, v.position);
                    maxV = glm::max(maxV, v.position);
                }
            }
        };

        accumulate(model->getOpaqueMeshes());
        accumulate(model->getTransparentMeshes());
        return (minV + maxV) * 0.5f;
    }
} // namespace

ShardBody::ShardBody(const char* name, const char* shardsFolder, Entity* entity, JPH::BodyInterface* bodyInterface) :
    m_name{name}, m_path{shardsFolder}, m_entity{entity}, m_bodyInterface{bodyInterface}
{
}

void ShardBody::init(void* engine)
{
    Engine* enginePtr{static_cast<Engine*>(engine)};
    m_models.clear();
    m_hulls.clear();
    m_numShards = 0;

    std::vector<std::filesystem::path> shardPaths{};
    for (const auto& entry : std::filesystem::directory_iterator(m_path))
    {
        const std::filesystem::path path{entry.path()};
        const std::string extension{path.extension().string()};
        if (extension == ".glb" || extension == ".gltf")
        {
            shardPaths.emplace_back(path);
        }
    }
    std::sort(shardPaths.begin(), shardPaths.end());

    for (const auto& path : shardPaths)
    {
        ++m_numShards;
        const std::string modelName{m_name + "_" + path.stem().string()};
        if (!enginePtr->modelExists(modelName))
        {
            enginePtr->addModel(modelName, path.string());
        }
        m_models.emplace_back(enginePtr->getModel(modelName));
    }

    const glm::vec3 scale{m_entity->getTransform().getLocalScale()};
    m_hulls.reserve(m_models.size());

    for (std::size_t i{0}; i < m_models.size(); ++i)
    {
        const Model* model{m_models[i]};
        const glm::vec3 shardCenter{getModelLocalCenter(model)};
        const std::string cachePath{"data/cache/" + m_name + "_" +
                                    std::filesystem::path(model->getPath()).stem().string() + ".bin"};

        ShapeLoader shapeLoader{};
        bool validCache{false};
        if (Util::fileExists(cachePath))
        {
            shapeLoader.loadFile(cachePath.c_str());
            JPH::Shape::ShapeResult* result{shapeLoader.getResult()};
            if (!result->HasError() && result->Get() != nullptr &&
                result->Get()->GetSubType() == JPH::EShapeSubType::ConvexHull)
            {
                validCache = true;
            }
        }

        if (!validCache)
        {
            std::vector<glm::vec3> vertices{};
            for (const Mesh* mesh : model->getOpaqueMeshes())
            {
                for (const MeshN::Vertex& v : mesh->getVertices())
                {
                    vertices.emplace_back((v.position - shardCenter) * scale);
                }
            }

            for (const Mesh* mesh : model->getTransparentMeshes())
            {
                for (const MeshN::Vertex& v : mesh->getVertices())
                {
                    vertices.emplace_back((v.position - shardCenter) * scale);
                }
            }

            shapeLoader = ShapeLoader{vertices};
            shapeLoader.exportFile(cachePath.c_str());
        }

        m_hulls.emplace_back(shapeLoader);
    }
}

void ShardBody::explode(const float force, std::vector<Entity*>& shards)
{
    if (m_broken)
        return;

    std::cout << "exploded!!\n";

    const Bounds::Transform& src{m_entity->getTransform()};
    const glm::vec3 scale{src.getLocalScale()};
    const glm::quat rotationQuat{glm::quat(glm::radians(src.getLocalRotation()))};

    shards.clear();
    shards.reserve(m_numShards);

    for (std::size_t i{0}; i < m_numShards; ++i)
    {
        const glm::vec3 shardCenter{getModelLocalCenter(m_models[i])};
        const glm::vec3 localOffset{shardCenter * scale};
        const glm::vec3 worldOffset{rotationQuat * localOffset};

        Bounds::Transform shardTransform{src};
        shardTransform.setLocalPosition(src.getGlobalPosition() + worldOffset);
        shardTransform.computeModelMatrix();

        Entity* shard{new Entity{m_models[i], shardTransform, BodyType::DYNAMIC, false}};

        const Bounds::Transform& finalT{shard->getTransform()};
        PhysicsBody physicsBody{m_hulls[i].getResult(), m_bodyInterface, finalT.getLocalRotation(), BodyType::DYNAMIC,
                                finalT.getGlobalPosition() + finalT.getPivotOffset()};
        shard->setPhysicsBody(physicsBody);

        if (force > 0.0f)
        {
            const glm::vec3 dir =
                glm::length(worldOffset) > 0.001f ? glm::normalize(worldOffset) : glm::vec3{0.0f, 1.0f, 0.0f};
            m_bodyInterface->AddImpulse(shard->getPhysicsBody()->getBodyID(),
                                        {dir.x * force, dir.y * force, dir.z * force});
        }

        shards.emplace_back(shard);
    }

    m_entity->setKill(true);
    m_broken = true;
}

std::pair<Model*, ShapeLoader*> ShardBody::getShard(const std::size_t idx)
{
    assert(idx < m_models.size() && idx < m_hulls.size());
    return {m_models[idx], &m_hulls[idx]};
}
