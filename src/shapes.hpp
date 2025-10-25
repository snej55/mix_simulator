#ifndef SHAPES_H
#define SHAPES_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "engine_types.hpp"
#include "shader.hpp"

// Generic rect template
template <typename T>
struct Rect
{
    T x{};
    T y{};
    T w{};
    T h{};
};

// use FRect most of the time
// floating point rect
typedef Rect<float> FRect;
// integer rect
typedef Rect<int> IRect;

struct Color
{
    int r{0};
    int g{0};
    int b{0};
    int a{255};
};

namespace Shapes
{
    inline float RectVertices[]{
        1.0f, 0.0f,  0.0f, // top right
        1.0f, -1.0f, 0.0f, // bottom right
        0.0f, -1.0f, 0.0f, // bottom left
        0.0f, 0.0f,  0.0f // top left
    };

    inline unsigned int RectIndices[]{
        0, 1, 3, // first Triangle
        1, 2, 3 // second Triangle
    };
} // namespace Shapes

class ShapeManager final : public EngineObject
{
public:
    explicit ShapeManager(EngineObject* parent);
    ~ShapeManager() override;

    // setup vertex buffers and stuff
    void init();

    // draw a rect
    template <typename T>
    void drawRect(const Rect<T>& rect, const Color& color, ShaderManager* shaderManager) const;
    void drawRect(const FRect& rect, const Color& color, ShaderManager* shader) const;
    // in case you want to draw an IRect for some reason
    void drawRect(const IRect& rect, const Color& color, ShaderManager* shader) const;

    [[nodiscard]] static glm::vec3 color2vec3(const Color& color);
    [[nodiscard]] static glm::vec4 color2vec4(const Color& color);

private:
    unsigned int m_rectVAO{}, m_rectVBO{}, m_rectEBO{};
};

#endif
