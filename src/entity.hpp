// Created by Jens Kromdijk  03-12-2025
// TODO: Add physics bodies
#ifndef ENTITY_H
#define ENTITY_H

#include <string>

#include "engine_types.hpp"
#include "model.hpp"

class Entity : public EngineObject
{
public:
    explicit Entity(EngineObject* parent, const std::string& entityName, const char* modelPath,
                    const glm::vec3& position, bool animated = false);
    ~Entity() override;

    void update(float deltaTime);

    // getters
    [[nodiscard]] const std::string& getName() const { return m_entityName; }
    [[nodiscard]] const std::string& getPath() const { return m_path; }
    [[nodiscard]] const glm::vec3& getPosition() const { return m_position; }

    [[nodiscard]] Model* getModel() const { return m_model; }
    [[nodiscard]] bool getAnimated() const { return m_animated; }

private:
    std::string m_entityName;
    std::string m_path;
    glm::vec3 m_position;

    Model* m_model{nullptr};
    bool m_animated{false};

    void loadModel(const char* modelPath);
    void freeModel();
};

#endif // ENTITY_H
