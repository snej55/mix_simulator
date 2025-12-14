// Created by Jens Kromdijk 04-12-2025

#include <JSON/json.hpp>
#include <array>
#include <cassert>
#include <iostream>

#include "scene.hpp"
#include "bounds.hpp"
#include "engine.hpp"
#include "spatial_hashing.hpp"
#include "util.hpp"

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

        if (entity->getDiscarded())
        {
            removeEntity(i);
            --i;
            continue;
        }

        if (entity->getDirty())
            continue;

        entity->update(deltaTime);
        entity->setDirty(true);

        // check if entity is still in chunk
        if (!entity->getTransform().getDirty())
            continue;

        if (!m_aabb->collidePoint(entity->getTransform().getGlobalPosition()))
        {
            entity->setDiscarded(true);
            discardEntities.emplace_back(entity);
            removeEntity(i); // swap entity with back and pop
            --i; // make sure swapped entity is not skipped
        }
    }
}

void SceneChunk::getVisible(const Bounds::Frustum& camFrustum, bool& visible) const
{
    visible = m_aabb->onFrustum(camFrustum, {}, SpatialHashing::CELL_PADDING);
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
    entity->setDiscarded(false);
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
    std::vector<Entity*> freedEntities;
    for (auto& [key, chunkPtr] : m_chunks)
    {
        const std::vector<Entity*>& entities{chunkPtr->getEntities()};
        for (Entity* entity : entities)
        {
            freedEntities.emplace_back(entity);
            if (std::find(freedEntities.begin(), freedEntities.end(), entity) == freedEntities.end())
                delete entity;
        }
    }
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
    for (const auto& [key, chunkPtr] : m_chunks)
    {
        bool visible{false};
        chunkPtr->getVisible(camFrustum, visible);
        if (visible)
        {
            chunks.emplace_back(chunkPtr.get());
        }
    }
}

void Scene::cleanupEmptyChunks()
{
    for (auto it{m_chunks.begin()}; it != m_chunks.end();)
    {
        if (it->second->getEntities().empty())
        {
            it = m_chunks.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Scene::addEntity(const char* modelPath, const Bounds::Transform& transform, const bool animated)
{
    assert(m_engine != nullptr);
    Engine* enginePtr{static_cast<Engine*>(m_engine)};

    Model* modelPtr{enginePtr->getModelByPath(modelPath)};
    if (modelPtr == nullptr)
    {
        Util::beginError();
        std::cout << "SCENE::ADD_ENTITY::ERROR: Model at path `" << modelPath
                  << "` does not exist! Ensure it is preloaded beforehand.";
        Util::endError();
        return;
    }

    if (animated)
    {
        modelPtr->loadAnimation();
    }

    Entity* entity{new Entity{modelPtr, transform, animated}};
    addEntity(entity);
}

void Scene::addEntity(Entity* entity)
{
    assert(entity != nullptr);

    // get iterator
    const SpatialHashing::ChunkKey key{getChunkKey(entity->getGlobalMidpoint())};

    constexpr std::array<glm::vec3, 9> neighbourOffsets{
        glm::vec3{-1.f, -1.f, -1.f}, glm::vec3{0.f, -1.f, -1.f}, glm::vec3{1.f, -1.f, -1.f},
        glm::vec3{-1.f, 0.f, -1.f},  glm::vec3{0.f, 0.f, -1.f},  glm::vec3{1.f, 0.f, -1.f},
        glm::vec3{-1.f, 1.f, -1.f},  glm::vec3{0.f, 1.f, -1.f},  glm::vec3{1.f, 1.f, -1.f}};

    for (const glm::vec3& offset : neighbourOffsets)
    {
        const SpatialHashing::ChunkKey neighbourKey{key.x + static_cast<long long>(offset.x),
                                                    key.y + static_cast<long long>(offset.y),
                                                    key.z + static_cast<long long>(offset.z)};
        if (m_chunks.find(neighbourKey) == m_chunks.end())
        {
            glm::vec3 chunkPos{static_cast<float>(neighbourKey.x) * SpatialHashing::CELL_SIZE,
                               static_cast<float>(neighbourKey.y) * SpatialHashing::CELL_SIZE,
                               static_cast<float>(neighbourKey.z) * SpatialHashing::CELL_SIZE};
            m_chunks[neighbourKey] = std::make_unique<SceneChunk>(chunkPos);
        }
        if (m_chunks[neighbourKey]->getAABB()->collideAABB(entity->getGlobalAABB()))
        {
            m_chunks[neighbourKey]->addEntity(entity);
        }
    }
}

SpatialHashing::ChunkKey Scene::getChunkKey(const glm::vec3& pos)
{
    SpatialHashing::ChunkKey key{};
    key.x = static_cast<long long>(std::floor(pos.x / SpatialHashing::CELL_SIZE));
    key.y = static_cast<long long>(std::floor(pos.y / SpatialHashing::CELL_SIZE));
    key.z = static_cast<long long>(std::floor(pos.z / SpatialHashing::CELL_SIZE));
    return key;
}
