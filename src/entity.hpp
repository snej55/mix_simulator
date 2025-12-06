// Created by Jens Kromdijk  03-12-2025
#ifndef ENTITY_H
#define ENTITY_H

#include <string>

#include "model.hpp"
#include "bounds.hpp"

class Entity
{
public:
    explicit Entity(const std::string& entityName, const char* modelPath, const glm::vec3& position,
                    bool animated = false);
    ~Entity();

    void update(float deltaTime);

    // getters
    [[nodiscard]] const std::string& getEntityName() const { return m_entityName; }
    [[nodiscard]] const std::string& getPath() const { return m_path; }
    [[nodiscard]] const glm::vec3& getPosition() const { return m_position; }

    [[nodiscard]] Model* getModel() const { return m_model; }
    [[nodiscard]] bool getAnimated() const { return m_animated; }

    [[nodiscard]] const Bounds::Transform& getTransform() const { return m_transform; }

private:
    std::string m_entityName;
    std::string m_path;
    glm::vec3 m_position;

    Model* m_model{nullptr};
    bool m_animated{false};

    Bounds::Transform m_transform;

    void loadModel(const char* modelPath);
    void freeModel();
};

#endif // ENTITY_H
