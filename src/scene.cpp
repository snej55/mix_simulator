// Created by Jens Kromdijk 04-12-2025

#include <cassert>
#include <unordered_set>
#include "scene.hpp"
#include "engine.hpp"

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
    if (animated && modelPtr != nullptr)
    {
        const_cast<Model*>(modelPtr)->loadAnimation();
    }
    std::cout << std::boolalpha << modelPtr->isAnimated() << "\n";

    Entity* entity{new Entity{modelPtr, transform, animated}};
    m_entities.push_back(entity);
}

void Scene::addEntity(Entity* entity) { m_entities.emplace_back(entity); }
