#ifndef POSTPROCESSING_H
#define POSTPROCESSING_H

#include "engine_types.hpp"
#include "shader.hpp"

class PostProcessor final : public EngineObject
{
public:
    explicit PostProcessor(EngineObject* parent);

    ~PostProcessor() override;

    // free resources
    void free();
    // check framebuffer
    [[nodiscard]] bool check() const;

    // initialize framebuffer and texture
    void init(int width, int height);
    // regenerate framebuffer for framebuffer_size_callback()
    void generate(int width, int height);

    // render framebuffer to screen
    void render(const Shader* screenShader) const;

    // bind framebuffer
    void enable() const;
    // unbind framebuffer
    void disable() const;

    // getters
    [[nodiscard]] int getWidth() const { return m_width; }
    [[nodiscard]] int getHeight() const { return m_height; }

    [[nodiscard]] unsigned int getFBO() const { return m_FBO; }
    [[nodiscard]] unsigned int getRBO() const { return m_RBO; }
    [[nodiscard]] unsigned int getTEX() const { return m_TEX; }

    [[nodiscard]] unsigned int getVAO() const { return m_VAO; }
    [[nodiscard]] unsigned int getVBO() const { return m_VBO; }

private:
    // framebuffer dimensions
    int m_width{0};
    int m_height{0};

    // framebuffer
    unsigned int m_FBO{};
    unsigned int m_RBO{};
    unsigned int m_TEX{};

    // simple quad
    unsigned int m_VAO{};
    unsigned int m_VBO{};

    void generateFramebuffer();
    void generateFramebufferTexture();
    void generateRenderbuffer();
    void generateQuad();
};

#endif
