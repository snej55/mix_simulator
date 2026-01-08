// Created by Jens Kromdijk 04-12-2025

// for scene parsing
#include <JSON/json.hpp>
#include "physics.hpp"
using json = nlohmann::json;

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

void SceneChunk::updateEntities(const float deltaTime, std::vector<Entity*>& discardEntities, JoltInstance* jolt)
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

        entity->update(deltaTime, jolt != nullptr ? jolt->getBodyInterface() : nullptr);
        entity->setDirty(true);

        // check if entity is still in chunk
        if (!entity->getTransform().getDirty())
            continue;

        if (!m_aabb->collideAABB(entity->getGlobalAABB()))
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

bool Scene::init(const char* scenePath)
{
    std::cout << "SCENE:::INIT: Loading scene from path `" << scenePath << "`...\n";
    std::ifstream sceneFile{scenePath};
    sceneFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        json data{json::parse(sceneFile)};

        // load models
        std::cout << "SCENE::INIT: Loading models for scene...\n";
        std::map<std::size_t, std::pair<std::string, std::string>> modelMap{};
        for (const auto& [key, value] : data[0]["level"]["models"].items())
        {
            const std::size_t modelID{std::stoul(key)};
            const std::string modelName{value["name"].get<std::string>()};
            const std::string modelPath{value["path"].get<std::string>()};
            std::cout << modelName << " " << modelPath << std::endl;

            modelMap[modelID] = std::pair{modelName, modelPath};

            // preload model into engine
            assert(m_engine != nullptr);
            Engine* enginePtr{static_cast<Engine*>(m_engine)};
            std::cout << "Adding model..." << std::endl;
            enginePtr->addModel(modelName, modelPath);
        }
        std::cout << "SCENE::INIT: Loaded " << modelMap.size() << " models for scene.\n";

        // load entities
        std::cout << "SCENE::INIT: Loading entities for scene...\n";
        for (const auto& entityEntry : data[0]["level"]["objects"])
        {
            const glm::vec3 position{entityEntry["position"][0].get<float>(), entityEntry["position"][1].get<float>(),
                                     entityEntry["position"][2].get<float>()};
            const glm::vec3 scale{entityEntry["scale"][0].get<float>(), entityEntry["scale"][1].get<float>(),
                                  entityEntry["scale"][2].get<float>()};
            const glm::vec3 rotation{entityEntry["rotation"][0].get<float>(), entityEntry["rotation"][1].get<float>(),
                                     entityEntry["rotation"][2].get<float>()};
            const std::size_t modelID{entityEntry["modelID"].get<std::size_t>()};
            const bool animated{entityEntry.value("animated", false)};

            const std::string_view bodyTypeStr{entityEntry.value("bodyType", "static")};
            BodyType bodyType;
            getBodyType(bodyTypeStr, &bodyType);

            Bounds::Transform transform{};
            transform.setLocalPosition(position);
            transform.setLocalScale(scale);
            transform.setLocalRotation(rotation);

            if (modelMap.find(modelID) == modelMap.end())
            {
                Util::beginError();
                std::cout << "SCENE::INIT::ERROR: Model ID `" << modelID << "` not found in model map!";
                Util::endError();
                continue;
            }

            addEntity(modelMap[modelID].second.c_str(), transform, bodyType, animated);
        }
        std::cout << "SCENE::INIT: Loaded " << data[0]["level"]["objects"].size() << " entities for scene.\n";
    }
    catch ([[maybe_unused]] const std::ifstream::failure& e)
    {
        Util::beginError();
        std::cout << "SCENE::INIT::ERROR: Could not read scene file at path `" << scenePath << "`!";
        Util::endError();
        return false;
    }

    sceneFile.close();
    return true;
}

void Scene::initPhysicsBodies(JoltInstance* jolt)
{
    for (auto& [key, chunkPtr] : m_chunks)
    {
        const std::vector<Entity*>& entities{chunkPtr->getEntities()};
        for (Entity* entity : entities)
        {
            if (entity->getPhysicsBody() == nullptr)
                entity->initPhysicsBody(jolt->getBodyInterface());
        }
    }
}

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
    m_chunks.clear();
}

void Scene::updateEntities(const float deltaTime, JoltInstance* jolt)
{
    std::vector<Entity*> discardEntities;

    for (const auto& [key, chunkPtr] : m_chunks)
    {
        chunkPtr->updateEntities(deltaTime, discardEntities, jolt);
    }

    for (Entity* entity : discardEntities)
    {
        addEntity(entity);
    }

    cleanupEmptyChunks();
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

void Scene::addEntity(const char* modelPath, const Bounds::Transform& transform, const BodyType& bodyType,
                      const bool animated)
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

    Entity* entity{new Entity{modelPtr, transform, bodyType, animated}};
    addEntity(entity);
}

void Scene::addEntity(Entity* entity)
{
    assert(entity != nullptr);

    // get iterator
    const SpatialHashing::ChunkKey key{getChunkKey(entity->getGlobalMidpoint())};

    constexpr std::array<glm::vec3, 27> neighbourOffsets{
        glm::vec3{-1.f, -1.f, -1.f}, glm::vec3{0.f, -1.f, -1.f}, glm::vec3{1.f, -1.f, -1.f}, glm::vec3{-1.f, 0.f, -1.f},
        glm::vec3{0.f, 0.f, -1.f},   glm::vec3{1.f, 0.f, -1.f},  glm::vec3{-1.f, 1.f, -1.f}, glm::vec3{0.f, 1.f, -1.f},
        glm::vec3{1.f, 1.f, -1.f},   glm::vec3{-1.f, -1.f, 0.f}, glm::vec3{0.f, -1.f, 0.f},  glm::vec3{1.f, -1.f, 0.f},
        glm::vec3{-1.f, 0.f, 0.f},   glm::vec3{0.f, 0.f, 0.f},   glm::vec3{1.f, 0.f, 0.f},   glm::vec3{-1.f, 1.f, 0.f},
        glm::vec3{0.f, 1.f, 0.f},    glm::vec3{1.f, 1.f, 0.f},   glm::vec3{-1.f, -1.f, 1.f}, glm::vec3{0.f, -1.f, 1.f},
        glm::vec3{1.f, -1.f, 1.f},   glm::vec3{-1.f, 0.f, 1.f},  glm::vec3{0.f, 0.f, 1.f},   glm::vec3{1.f, 0.f, 1.f},
        glm::vec3{-1.f, 1.f, 1.f},   glm::vec3{0.f, 1.f, 1.f},   glm::vec3{1.f, 1.f, 1.f}};

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
            // std::cout << "Adding entity to chunk at key " << neighbourKey << "\n";
            SceneChunk* chunkPtr{m_chunks[neighbourKey].get()};
            entity->addChunk(chunkPtr, chunkPtr->getNumEntities()); // gets implicitly cast from SceneChunk* to void*
            chunkPtr->addEntity(entity);
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
