#ifndef POSTPROCESSING_H
#define POSTPROCESSING_H

#include "engine_types.hpp"
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
} // namespace PostProcessingN

class BloomFBO final : public EngineObject
{
public:
    explicit BloomFBO(EngineObject* parent);
    ~BloomFBO() override;

    bool init(unsigned int width, unsigned int height, unsigned int mipChainLength);
    void bind() const;
    void free();

    [[nodiscard]] const std::vector<PostProcessingN::BloomMip>& mipChain() const { return m_mipChain; }

private:
    bool m_init{false};
    unsigned int m_FBO{};
    std::vector<PostProcessingN::BloomMip> m_mipChain{};
};

class BloomRenderer final : public EngineObject
{
public:
    explicit BloomRenderer(EngineObject* parent);
    ~BloomRenderer() override;

    bool init(unsigned int width, unsigned int height, void* engine);
    void free();
    void renderBloomTexture(unsigned int srcTexture, float filterRadius) const;

    [[nodiscard]] unsigned int bloomTexture() const { return m_FBO.mipChain()[0].texture; }

private:
    BloomFBO m_FBO;

    bool m_init{false};
    glm::ivec2 m_srcViewportSize{};
    glm::vec2 m_srcViewportSizeF{};

    Shader* m_downSampleShader{nullptr};
    Shader* m_upSampleShader{nullptr};

    unsigned int m_quadVAO{0}, m_quadVBO{0};

    void renderDownSamples(unsigned int srcTexture) const;
    void renderUpSamples(float filterRadius) const;

    void setupQuad();
};

#define SSAO_KERNEL_SIZE 24
#define SSAO_NOISE_SIZE 4
#define SSAO_SCALE 2

class SSAOGenerator final : public EngineObject
{
public:
    explicit SSAOGenerator(EngineObject* parent);
    ~SSAOGenerator() override;

    void init(int width, int height);
    void free();

    void render(void* dfRenderer, const Shader* ssaoShader, const glm::mat4& projection, const Shader* ssaoBlurShader,
                const glm::mat4& view) const;

    void renderQuad() const;

    // getters
    [[nodiscard]] bool getInit() const { return m_init; }
    [[nodiscard]] int getWidth() const { return m_width; }
    [[nodiscard]] int getHeight() const { return m_height; }

    [[nodiscard]] unsigned int getNoiseTexture() const { return m_noiseTexture; }
    [[nodiscard]] unsigned int getSSAO_FBO() const { return m_ssaoFBO; }
    [[nodiscard]] unsigned int getSSAO_FBOBlur() const { return m_ssaoFBOBlur; }
    [[nodiscard]] unsigned int getSSAO_ColorBuffer() const { return m_ssaoColorBuffer; }
    [[nodiscard]] unsigned int getSSAO_ColorBufferBlur() const { return m_ssaoColorBufferBlur; }

private:
    bool m_init{false};
    int m_width{0};
    int m_height{0};

    unsigned int m_noiseTexture{};
    unsigned int m_ssaoFBO{};
    unsigned int m_ssaoFBOBlur{};
    unsigned int m_ssaoColorBuffer{};
    unsigned int m_ssaoColorBufferBlur{};

    std::vector<glm::vec3> m_ssaoKernel{};
    std::vector<glm::vec3> m_ssaoNoise{};

    // quad vertex array object
    unsigned int m_quadVAO{};
    unsigned int m_quadVBO{};

    void initQuad();
    void freeQuad() const;
};


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
    void generate(int width, int height, void* engine);

    // render framebuffer to screen
    void renderHDR(const Shader* screenShader) const;
    void renderFinal(const Shader* fxaaShader, unsigned int* uiTEX = nullptr) const;

    // bind framebuffer
    void enable() const;
    // unbind framebuffer
    void disable() const;

    // toggle bloom
    void enableBloom(void* engine);
    void disableBloom();

    // getters
    [[nodiscard]] int getWidth() const { return m_width; }
    [[nodiscard]] int getHeight() const { return m_height; }

    [[nodiscard]] unsigned int getFBO() const { return m_FBO; }
    [[nodiscard]] unsigned int getRBO() const { return m_RBO; }
    [[nodiscard]] unsigned int getTEX() const { return m_TEX; }

    [[nodiscard]] unsigned int getVAO() const { return m_VAO; }
    [[nodiscard]] unsigned int getVBO() const { return m_VBO; }

    [[nodiscard]] unsigned int getFXAA_FBO() const { return m_fxaaFBO; }
    [[nodiscard]] unsigned int getFXAA_TEX() const { return m_fxaaTex; }

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

    unsigned int m_fxaaFBO{};
    unsigned int m_fxaaTex{};

    bool m_bloomEnabled{false};
    BloomRenderer* m_bloomRenderer{nullptr};

    void generateFramebuffer();
    void generateFramebufferTexture();
    void generateRenderbuffer();
    void generateQuad();
};

#endif
