#ifndef POSTPROCESSING_H
#define POSTPROCESSING_H

#include "engine_types.hpp"
#include "glm/ext/vector_int2.hpp"
#include "shader.hpp"

#include <vector>

namespace PostProcessingN
{
    struct BloomMip
    {
	glm::vec2 size;
	glm::ivec2 intSize;
	unsigned int texture;
    };
}

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

class BloomFBO final : public EngineObject
{
public:
    explicit BloomFBO(EngineObject* parent);
    ~BloomFBO() override;

    bool init(unsigned int width, unsigned int height, unsigned int mipChainLength);
    void bind();
    void free();  

    [[nodiscard]] const std::vector<PostProcessingN::BloomMip>& mipChain() const {return m_mipChain;}

private:
    bool m_init{false};
    unsigned int m_FBO{};
    std::vector<PostProcessingN::BloomMip> m_mipChain{};
};

class BloomRenderer final : public EngineObject
{
public:
    explicit BloomRenderer(EngineObject* parent);
    ~BloomRenderer();

    bool init(unsigned int width, unsigned int height, void* engine);
    void free();
    void renderBloomTexture(unsigned int srcTexture, float filterRadius);

    [[nodiscard]] unsigned int bloomTexture() const {return m_FBO.mipChain()[0].texture;}

private:
    BloomFBO m_FBO;

    bool m_init{false};
    glm::ivec2 m_srcViewportSize{};
    glm::vec2 m_srcViewportSizeF{};

    Shader* m_downSampleShader{nullptr};
    Shader* m_upSampleShader{nullptr};

    unsigned int m_quadVAO{0}, m_quadVBO{0};

    void renderDownSamples(unsigned int srcTexture);
    void renderUpSamples(float filterRadius);

    void setupQuad();
};

#endif
