// Created by Jens Kromdijk 04-12-2025

#include <JSON/json.hpp>
#include <cassert>

#include "scene.hpp"
#include "bounds.hpp"
#include "engine.hpp"
#include "spatial_hashing.hpp"

SceneChunk::SceneChunk(const glm::vec3& pos) : m_pos{pos} { init(); }

SceneChunk::SceneChunk(const glm::vec3& pos, const std::vector<Entity*>& entities) : m_pos{pos}, m_entities{entities}
{
    init();
}

void SceneChunk::updateEntities(const float deltaTime, std::vector<Entity*>& discardEntities)
{
    for (std::size_t i{0}; i < m_entities.size(); ++i)
    {
        Entity* entity{m_entities[i]};
        assert(entity != nullptr);
        entity->update(deltaTime);
        if (!m_aabb->collidePoint(entity->getTransform().getGlobalPosition()))
        {
            discardEntities.emplace_back(entity);
            removeEntity(i);
        }
    }
}

void SceneChunk::removeEntity(const std::size_t index)
{
    assert(index < m_entities.size());
    if (m_entities.size() == 1)
    {
        m_entities.clear();
        return;
    }

    std::swap(m_entities[index], m_entities.back());
    m_entities.pop_back();
}

void SceneChunk::addEntity(Entity* entity)
{
    assert(entity != nullptr);
    m_entities.emplace_back(entity);
}

void SceneChunk::init()
{
    m_aabb = std::make_unique<Bounds::AABB>(
        Bounds::AABB{m_pos,
                     {m_pos.x + SpatialHashing::CELL_SIZE, m_pos.y + SpatialHashing::CELL_SIZE,
                      m_pos.z + SpatialHashing::CELL_SIZE}});
}

Scene::Scene(void* engine) : EngineObject{"Scene", static_cast<EngineObject*>(engine)}, m_engine(engine) {}

Scene::~Scene() { free(); }

bool Scene::init(const char* scenePath) { return true; }

void Scene::free()
{
    for (std::size_t i{0}; i < std::size(m_entities); ++i)
    {
        delete m_entities[i];
    }
    m_entities.clear();
}

void Scene::updateEntities(const float deltaTime)
{
    std::vector<Entity*> discardEntities;

    for (const auto& [key, chunkPtr] : m_chunks)
    {
        chunkPtr->updateEntities(deltaTime, discardEntities);
    }

    for (Entity* entity : discardEntities)
    {
        addEntity(entity);
    }
}

void Scene::getVisibleChunks(const Bounds::Frustum& camFrustum, const Bounds::AABB& frustumBV,
                             std::vector<SceneChunk*>& chunks)
{
    SpatialHashing::ChunkKey minKey{getChunkKey(frustumBV.center - frustumBV.extents)};
    SpatialHashing::ChunkKey maxKey{getChunkKey(frustumBV.center + frustumBV.extents)};

    for (long long x{minKey.x}; x <= maxKey.x; ++x)
    {
        for (long long y{minKey.y}; y <= maxKey.y; ++y)
        {
            for (long long z{minKey.z}; z < maxKey.z; ++z)
            {
                SpatialHashing::ChunkKey key{x, y, z};
                auto it{m_chunks.find(key)};
                if (it != m_chunks.end())
                {
                    chunks.emplace_back(it->second.get());
                    // TODO: Add chunk visibility check here
                }
            }
        }
    }
}

void Scene::addEntity(const char* modelPath, const Bounds::Transform& transform, const bool animated)
{
    assert(m_engine != nullptr);
    const Engine* enginePtr{static_cast<Engine*>(m_engine)};

    const Model* modelPtr{enginePtr->getModelByPath(modelPath)};
    if (modelPtr == nullptr)
    {
        enginePtr->addModel(modelPath, modelPath);
        modelPtr = enginePtr->getModelByPath(modelPath);
    }
    assert(modelPtr != nullptr);

    if (animated && modelPtr != nullptr)
    {
        const_cast<Model*>(modelPtr)->loadAnimation();
    }
    std::cout << std::boolalpha << modelPtr->isAnimated() << "\n";

    Entity* entity{new Entity{modelPtr, transform, animated}};
    addEntity(entity);
}

void Scene::addEntity(Entity* entity)
{
    assert(entity != nullptr);

    // get iterator
    SpatialHashing::ChunkKey key{getChunkKey(entity->getTransform().getGlobalPosition())};
    auto it{m_chunks.find(key)};
    if (it != m_chunks.end())
    {
        it->second->addEntity(entity);
    }
    else
    {
        glm::vec3 chunkPos{key.x * SpatialHashing::CELL_SIZE, key.y * SpatialHashing::CELL_SIZE,
                           key.z * SpatialHashing::CELL_SIZE};
        m_chunks[key] = std::make_unique<SceneChunk>(chunkPos, std::vector<Entity*>{entity});
    }
}

SpatialHashing::ChunkKey Scene::getChunkKey(const glm::vec3& pos) const
{
    SpatialHashing::ChunkKey key{};
    key.x = static_cast<long long>(std::floor(pos.x / SpatialHashing::CELL_SIZE));
    key.y = static_cast<long long>(std::floor(pos.y / SpatialHashing::CELL_SIZE));
    key.z = static_cast<long long>(std::floor(pos.z / SpatialHashing::CELL_SIZE));
    return key;
}
