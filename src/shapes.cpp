//
// Created by Jens Kromdijk on 22/06/25.
//

#include <glad/glad.h>

#include <glm/ext/matrix_transform.hpp>

#include "shapes.hpp"
#include "util.hpp"

ShapeManager::ShapeManager(EngineObject* parent) : EngineObject{"ShapeManager", parent}
{
    // generate vertex arrays
    init();
}

ShapeManager::~ShapeManager()
{
    glDeleteVertexArrays(1, &m_rectVAO);
    glDeleteBuffers(1, &m_rectVBO);
    glDeleteBuffers(1, &m_rectEBO);
}

// generate vertex arrays
void ShapeManager::init()
{
    glGenVertexArrays(1, &m_rectVAO);
    glGenBuffers(1, &m_rectVBO);
    glGenBuffers(1, &m_rectEBO);

    glBindVertexArray(m_rectVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Shapes::RectVertices), Shapes::RectVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rectEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Shapes::RectIndices), Shapes::RectIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // we can now safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// draw a rect
template <typename T>
void ShapeManager::drawRect(const Rect<T>& rect, const Color& color, ShaderManager* shaderManager) const
{
    // get rect position and dimensions
    glm::mat4 model{1.0f};
    model = glm::translate(model, glm::vec3(rect.x, rect.y, 0.0f));
    model = glm::scale(model, glm::vec3(rect.w, rect.h, 1.0f));

    // get shader from shader manager
    const Shader* rectShader{shaderManager->getShader("rect")};
    if (rectShader == nullptr)
    {
        Util::beginError();
        std::cout << "SHAPE_MANAGER::DRAW_RECT::ERROR: Could not find shader *rect*!";
        Util::endError();
        return;
    }

    // activate shader
    rectShader->use();
    rectShader->setMat4("model", model);
    rectShader->setVec3("shapeColor", color2vec3(color));
    // render rect
    glBindVertexArray(m_rectVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void ShapeManager::drawRect(const FRect& rect, const Color& color, ShaderManager* shaderManager) const
{
    drawRect<float>(rect, color, shaderManager);
}

void ShapeManager::drawRect(const IRect& rect, const Color& color, ShaderManager* shaderManager) const
{
    drawRect<int>(rect, color, shaderManager);
}


// rgba color (0-255) to floating point (0.0-1.0) opengl colours
glm::vec3 ShapeManager::color2vec3(const Color& color)
{
    return {static_cast<float>(color.r) / 255.0f, static_cast<float>(color.g) / 255.0f,
            static_cast<float>(color.b) / 255.0f};
}

glm::vec4 ShapeManager::color2vec4(const Color& color)
{
    return {static_cast<float>(color.r) / 255.0f, static_cast<float>(color.g) / 255.0f,
            static_cast<float>(color.b) / 255.0f, static_cast<float>(color.a) / 255.0f};
}
