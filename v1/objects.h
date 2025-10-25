#ifndef OBJECTS_H
#define OBJECTS_H

#include <glad/glad.h>
#include "engine.h"

#include <iostream>

class Object : public EngineObject
{
public:
    explicit Object(EngineObject* parent);
    ~Object() override; // destroy VAO, VBO

    // load vertex array objects & vertex buffer objects
    virtual void init(const std::vector<float>& vertices);
    void free() override;

    // getters & setters
    [[nodiscard]] unsigned int getVAO() const {return m_VAO;}
    [[nodiscard]] unsigned int getVBO() const {return m_VBO;}
    [[nodiscard]] unsigned int getEBO() const {return m_EBO;}

private:
    unsigned int m_VAO{0};
    unsigned int m_VBO{0};
    unsigned int m_EBO{0};
};

#endif