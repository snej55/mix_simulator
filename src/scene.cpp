#include "scene.hpp"

Scene::Scene(EngineObject* parent) : EngineObject{"Scene", parent} {}

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

void Scene::addEntity(Entity* entity) { m_entities.emplace_back(entity); }
