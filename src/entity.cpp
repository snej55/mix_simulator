// Created by Jens Kromdijk 03-12-2025

#include "entity.hpp"

Entity::Entity(EngineObject* parent, const std::string& entityName, const char* modelPath, const glm::vec3& position,
               const bool animated) :
    EngineObject{entityName.c_str(), parent}, m_entityName{entityName}, m_path{modelPath}, m_position{position}
{
    loadModel(modelPath);
}

Entity::~Entity() { freeModel(); }

void Entity::update(const float deltaTime)
{
    if (m_animated && m_model != nullptr)
    {
        m_model->updateAnimation(deltaTime);
    }
}

void Entity::loadModel(const char* modelPath)
{
    if (m_model != nullptr)
    {
        freeModel();
        m_model = nullptr;
    }

    // we want to own the model
    m_model = new Model{modelPath, this};
    m_model->loadModel(modelPath);

    if (m_animated)
    {
        m_model->loadAnimation();
    }
}

void Entity::freeModel()
{
    if (m_model != nullptr)
    {
        delete m_model;
        m_model = nullptr;
    }
}
