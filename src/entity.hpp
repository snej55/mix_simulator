// Created by Jens Kromdijk  03-12-2025
#ifndef ENTITY_H
#define ENTITY_H

#include <memory>
#include <string>

#include "model.hpp"
#include "bounds.hpp"

class Entity
{
public:
    Entity(const std::string& entityName, const char* modelPath, const Bounds::Transform& transform,
           bool animated = false);
    ~Entity();

    void update(float deltaTime);

    [[nodiscard]] Bounds::AABB getGlobalAABB() const;

    [[nodiscard]] const std::string& getEntityName() const { return m_entityName; }
    [[nodiscard]] const std::string& getPath() const { return m_path; }
    [[nodiscard]] glm::vec3 getPosition() const { return m_transform.getGlobalPosition(); }

    [[nodiscard]] Model* getModel() const { return m_model; }
    [[nodiscard]] bool getAnimated() const { return m_animated; }

    [[nodiscard]] const Bounds::Transform& getTransform() const { return m_transform; }
    [[nodiscard]] bool getInFrustum() const { return m_inFrustum; }

private:
    std::string m_entityName;
    std::string m_path;
    Bounds::Transform m_transform;
    std::unique_ptr<Bounds::AABB> m_BV;

    Model* m_model{nullptr};
    bool m_animated{false};

    bool m_inFrustum{false};

    void loadModel(const char* modelPath);
    void freeModel();
};

#endif // ENTITY_H
