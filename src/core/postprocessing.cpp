#include "postprocessing.hpp"

#include "engine.hpp"
#include "engine_types.hpp"
#include "renderer.hpp"
#include "util.hpp"
#include "shapes.hpp"

PostProcessor::PostProcessor(EngineObject* parent) : EngineObject{"PostProcessor", parent} {}

PostProcessor::~PostProcessor() { free(); }

// free framebuffer
void PostProcessor::free()
{
    disableBloom();
    glDeleteTextures(1, &m_TEX);
    glDeleteRenderbuffers(1, &m_RBO);
    glDeleteFramebuffers(1, &m_FBO);

    glDeleteTextures(1, &m_fxaaTex);
    glDeleteFramebuffers(1, &m_fxaaFBO);
}

// check framebuffer
bool PostProcessor::check() const
{
    // check
    bool success{true};
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    // check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Util::beginError();
        std::cout << "POST_PROCESSOR::CHECK::ERROR: Framebuffer is not complete!";
        Util::endError();
        success = false;
    }
    else
    {
        std::cout << "Successfully initialized postprocessor!" << std::endl;
    }
    // unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return success;
}


// initialize framebuffer
void PostProcessor::init(const int width, const int height)
{
    m_width = width;
    m_height = height;

    // generate framebuffer
    generateFramebuffer();
    generateFramebufferTexture();
    generateRenderbuffer();

    // check framebuffer
    if (!check())
    {
        Util::beginError();
        std::cout << "POST_PROCESSOR::INIT::ERROR: Error checking framebuffer!";
        Util::endError();
    }

    // FXAA framebuffer
    glGenFramebuffers(1, &m_fxaaFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fxaaFBO);
    // create texture to apply FXAA on
    glGenTextures(1, &m_fxaaTex);
    glBindTexture(GL_TEXTURE_2D, m_fxaaTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // attach to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fxaaTex, 0);
    // check framebuffer
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Util::beginError();
        std::cout << "POST_PROCESSOR::INIT::ERROR: FXAA Framebuffer is not complete!";
        Util::endError();
    }

    // create quad
    generateQuad();
}

void PostProcessor::renderHDR(const Shader* screenShader) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fxaaFBO);
    glViewport(0, 0, m_width, m_height);

    glDisable(GL_DEPTH_TEST);
    // clear buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    screenShader->use();
    screenShader->setInt("screenTexture", 0);
    if (m_bloomEnabled)
    {
        assert(m_bloomRenderer != nullptr);
        m_bloomRenderer->renderBloomTexture(m_TEX, 0.005f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_bloomRenderer->bloomTexture());
        screenShader->use();
        screenShader->setInt("bloomBlur", 1);
    }

    glBindVertexArray(m_VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TEX);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::renderFinal(const Shader* fxaaShader, unsigned int* uiTEX) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    // clear buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    fxaaShader->use();
    fxaaShader->setInt("screenTexture", 0);
    fxaaShader->setInt("scrWidth", m_width);
    fxaaShader->setInt("scrHeight", m_height);

    if (uiTEX != nullptr)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, *uiTEX);
        fxaaShader->setInt("uiTexture", 1);
        fxaaShader->setInt("uiEnabled", 1);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fxaaTex);
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void PostProcessor::generate(const int width, const int height, void* engine)
{
    const bool bloomEnabled{m_bloomEnabled};
    free();
    init(width, height);

    if (bloomEnabled)
        enableBloom(engine);
}

void PostProcessor::generateFramebuffer()
{
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    m_FBO = framebuffer;
}

void PostProcessor::generateFramebufferTexture()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    // generate texture
    unsigned int textureColorBuffer;
    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    // Allocate HDR texture with correct data type
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // attach to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
    m_TEX = textureColorBuffer;
}

void PostProcessor::generateRenderbuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    // generate render buffer object
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, m_width, m_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
    m_RBO = rbo;
}

void PostProcessor::generateQuad()
{
    constexpr float quadVerticesTexCoords[]{-1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
                                            -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerticesTexCoords), quadVerticesTexCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));

    m_VAO = quadVAO;
    m_VBO = quadVBO;

    glBindVertexArray(0);
}

void PostProcessor::enable() const { glBindFramebuffer(GL_FRAMEBUFFER, m_FBO); }

void PostProcessor::disable() const
{
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::enableBloom(void* engine)
{
    if (m_bloomEnabled)
        return;

    m_bloomRenderer = new BloomRenderer{this};
    if (!m_bloomRenderer->init(m_width, m_height, engine))
    {
        delete m_bloomRenderer;
        Util::beginError();
        std::cout << "POST_PROCESSOR::ENABLE_BLOOM::ERROR: Failed to initialize bloom renderer!" << std::endl;
        Util::endError();
        return;
    }

    m_bloomEnabled = true;
}

void PostProcessor::disableBloom()
{
    if (!m_bloomEnabled)
        return;

    delete m_bloomRenderer;
    m_bloomRenderer = nullptr;
    m_bloomEnabled = false;
}

BloomFBO::BloomFBO(EngineObject* parent) : EngineObject{"BloomFBO", parent} {}

BloomFBO::~BloomFBO() { free(); }

// free resources
void BloomFBO::free()
{
    if (!m_init)
        return;

    for (std::size_t i{0}; i < m_mipChain.size(); ++i)
    {
        glDeleteTextures(1, &m_mipChain[i].texture);
    }
    m_mipChain.clear();
    glDeleteFramebuffers(1, &m_FBO);
    m_FBO = 0;
    m_init = false;
}

bool BloomFBO::init(const unsigned int width, const unsigned int height, const unsigned int mipChainLength)
{
    if (m_init)
        return true;

    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    glm::vec2 mipSize{static_cast<float>(width), static_cast<float>(height)};
    glm::ivec2 mipIntSize{static_cast<int>(width), static_cast<int>(height)};

    // check for overflow (safety check)
    if (width > static_cast<unsigned int>(INT_MAX) || height > static_cast<unsigned int>(INT_MAX))
    {
        Util::beginError();
        std::cout << "BLOOM_FBO::INIT::ERROR: Window size conversion overflow - cannot build bloom FBO!" << std::endl;
        Util::endError();
        return false;
    }

    for (unsigned int i{0}; i < mipChainLength; ++i)
    {
        PostProcessingN::BloomMip mip{};

        mipSize *= 0.5f;
        mipIntSize /= 2;
        mip.size = mipSize;
        mip.intSize = mipIntSize;

        glGenTextures(1, &mip.texture);
        glBindTexture(GL_TEXTURE_2D, mip.texture);
        // MOTE: hdr color format
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, static_cast<int>(mipSize.x), static_cast<int>(mipSize.y), 0,
                     GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        m_mipChain.emplace_back(mip);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_mipChain[0].texture, 0);

    // setup attachments
    constexpr unsigned int attachments[1]{GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, attachments);

    // check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Util::beginError();
        std::cout << "BLOOM_FBO::INIT::ERROR: Framebuffer is incomplete!" << std::endl;
        Util::endError();
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_init = true;
    return true;
}

void BloomFBO::bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_FBO); }

BloomRenderer::BloomRenderer(EngineObject* parent) : EngineObject{"BloomRenderer", parent}, m_FBO{this} {}

BloomRenderer::~BloomRenderer() { free(); }

void BloomRenderer::free()
{
    if (!m_init)
        return;

    glDeleteBuffers(1, &m_quadVBO);
    glDeleteVertexArrays(1, &m_quadVAO);
    m_FBO.free();
    m_init = false;
}

bool BloomRenderer::init(const unsigned int width, const unsigned int height, void* engine)
{
    if (m_init)
        return true;

    m_srcViewportSize = glm::ivec2{width, height};
    m_srcViewportSizeF = glm::vec2{static_cast<float>(width), static_cast<float>(height)};

    constexpr unsigned int numMips{5};
    if (!m_FBO.init(width, height, numMips))
    {
        Util::beginError();
        std::cout << "BLOOM_RENDERER::INIT::ERROR: Failed to create Bloom FBO!" << std::endl;
        Util::endError();
        return false;
    }

    const Engine* enginePtr{static_cast<Engine*>(engine)};
    m_downSampleShader = enginePtr->getShader("downSample");
    if (m_downSampleShader == nullptr)
    {
        Util::beginError();
        std::cout << "BLOOM_RENDERER::INIT::ERROR: Could not find down sample shader!" << std::endl;
        Util::endError();
        return false;
    }

    m_upSampleShader = enginePtr->getShader("upSample");
    if (m_upSampleShader == nullptr)
    {
        Util::beginError();
        std::cout << "BLOOM_RENDERER:INIT::ERROR: Could not find up sample shader!" << std::endl;
        Util::endError();
        return false;
    }

    m_downSampleShader->use();
    m_downSampleShader->setInt("tex", 0);
    glUseProgram(0);

    m_upSampleShader->use();
    m_upSampleShader->setInt("tex", 0);
    glUseProgram(0);

    // setup quad VAO
    setupQuad();

    m_init = true;
    return true;
}

void BloomRenderer::renderBloomTexture(const unsigned int srcTexture, const float filterRadius) const
{
    GLint origFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &origFBO);
    GLint origViewport[4];
    glGetIntegerv(GL_VIEWPORT, origViewport);
    m_FBO.bind();

    renderDownSamples(srcTexture);
    renderUpSamples(filterRadius);

    glBindFramebuffer(GL_FRAMEBUFFER, origFBO);
    glViewport(origViewport[0], origViewport[1], origViewport[2], origViewport[3]);
}

// downsample source texture
void BloomRenderer::renderDownSamples(const unsigned int srcTexture) const
{
    const std::vector<PostProcessingN::BloomMip>& mipChain{m_FBO.mipChain()};

    m_downSampleShader->use();
    m_downSampleShader->setVec2("srcResolution", m_srcViewportSizeF);

    // bind source texture (HDR color buffer) as initial texture input
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexture);

    // progressively downsample through mip chain
    for (std::size_t i{0}; i < mipChain.size(); ++i)
    {
        m_downSampleShader->setInt("mipLevel", static_cast<int>(i));
        const PostProcessingN::BloomMip& mip{mipChain[i]};
        glViewport(0, 0, static_cast<int>(mip.size.x), static_cast<int>(mip.size.y));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);

        glBindVertexArray(m_quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // setup current mip as input for next iteration
        m_downSampleShader->setVec2("srcResolution", mip.size);
        glBindTexture(GL_TEXTURE_2D, mip.texture);
    }
    glUseProgram(0);
}

// upsample source texture
void BloomRenderer::renderUpSamples(const float filterRadius) const
{
    const std::vector<PostProcessingN::BloomMip>& mipChain{m_FBO.mipChain()};

    m_upSampleShader->use();
    m_upSampleShader->setFloat("filterRadius", filterRadius);

    // additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    for (std::size_t i{mipChain.size() - 1}; i > 0; --i)
    {
        const PostProcessingN::BloomMip& mip{mipChain[i]};
        const PostProcessingN::BloomMip& nextMip{mipChain[i - 1]};

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mip.texture);

        glViewport(0, 0, static_cast<int>(nextMip.size.x), static_cast<int>(nextMip.size.y));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nextMip.texture, 0);

        glBindVertexArray(m_quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    glDisable(GL_BLEND);
    glUseProgram(0);
}

void BloomRenderer::setupQuad()
{
    if (m_quadVAO == 0)
    {
        glGenVertexArrays(1, &m_quadVAO);
        glGenBuffers(1, &m_quadVBO);

        glBindVertexArray(m_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Shapes::QuadVertices), Shapes::QuadVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }
}

SSAOGenerator::SSAOGenerator(EngineObject* parent) : EngineObject{"SSAOGenerator", parent} { initQuad(); }

SSAOGenerator::~SSAOGenerator()
{
    free();
    freeQuad();
}

void SSAOGenerator::init(const int width, const int height)
{
    if (m_init)
        free();

    if (m_quadVAO == 0 || m_quadVBO == 0)
    {
        initQuad();
    }

    // generate framebuffers
    glGenFramebuffers(1, &m_ssaoFBO);
    glGenFramebuffers(1, &m_ssaoFBOBlur);
    glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoFBO);

    // color buffer
    glGenTextures(1, &m_ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width / SSAO_SCALE, height / SSAO_SCALE, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Util::beginError();
        std::cout << "SSAO_GENERATOR::INIT::ERROR: Failed to create framebuffer!" << std::endl;
        Util::endError();
    }

    // same for blur color buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoFBOBlur);
    glGenTextures(1, &m_ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, m_ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Util::beginError();
        std::cout << "SSAO_GENERATOR::INIT::ERROR: Failed to create framebuffer!" << std::endl;
        Util::endError();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // generate random kernel
    std::uniform_real_distribution<GLfloat> randomF{0.0f, 1.0f};
    std::default_random_engine generator;
    m_ssaoKernel.clear();
    m_ssaoKernel.reserve(SSAO_KERNEL_SIZE);
    for (unsigned int i{0}; i < SSAO_KERNEL_SIZE; ++i)
    {
        // x & y from -1.0 - 1.0, z from 0.0 to 1.0 (-1.0 to 1.0 for z is a full sphere, we want a hemisphere)
        glm::vec3 sample{randomF(generator) * 2.0f - 1.0f, randomF(generator) * 2.0f - 1.0f, randomF(generator)};
        sample = glm::normalize(sample);
        sample *= randomF(generator);
        float scale{static_cast<float>(i) / static_cast<float>(SSAO_KERNEL_SIZE)};

        scale = Util::lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        m_ssaoKernel.emplace_back(sample);
    }

    // generate noise texture - larger for better sampling
    m_ssaoNoise.clear();
    m_ssaoNoise.reserve(SSAO_NOISE_SIZE * SSAO_NOISE_SIZE);
    for (unsigned int i{0}; i < SSAO_NOISE_SIZE * SSAO_NOISE_SIZE; ++i)
    {
        glm::vec3 noise{randomF(generator) * 2.0f - 1.0f, randomF(generator) * 2.0f - 1.0f, 0.0f};
        m_ssaoNoise.emplace_back(glm::normalize(noise));
    }

    glGenTextures(1, &m_noiseTexture);
    glBindTexture(GL_TEXTURE_2D, m_noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SSAO_NOISE_SIZE, SSAO_NOISE_SIZE, 0, GL_RGB, GL_FLOAT, &m_ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    m_width = width;
    m_height = height;
    m_init = true;
}

void SSAOGenerator::free()
{
    if (m_init)
    {
        glDeleteTextures(1, &m_noiseTexture);
        glDeleteTextures(1, &m_ssaoColorBuffer);
        glDeleteFramebuffers(1, &m_ssaoFBO);
        glDeleteTextures(1, &m_ssaoColorBufferBlur);
        glDeleteFramebuffers(1, &m_ssaoFBOBlur);

        // freeQuad();

        m_init = false;
    }
}

void SSAOGenerator::renderQuad() const
{
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// setup quad for rendering
void SSAOGenerator::initQuad()
{
    constexpr float quadVertices[]{
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
    };
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
}

void SSAOGenerator::freeQuad()
{
    glDeleteBuffers(1, &m_quadVBO);
    glDeleteVertexArrays(1, &m_quadVAO);
    m_quadVAO = 0;
    m_quadVBO = 0;
}

void SSAOGenerator::render(void* dfRenderer, const Shader* ssaoShader, const glm::mat4& projection,
                           const Shader* ssaoBlurShader, const glm::mat4& view) const
{
    assert(dfRenderer != nullptr);
    assert(ssaoShader != nullptr);
    assert(ssaoBlurShader != nullptr);
    const DeferredRenderer* dfRendererPtr{static_cast<DeferredRenderer*>(dfRenderer)};

    // SSAO Pass
    glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoFBO);
    glViewport(0, 0, m_width / SSAO_SCALE, m_height / SSAO_SCALE);
    glClear(GL_COLOR_BUFFER_BIT);
    ssaoShader->use();

    // send kernel samples
    for (std::size_t i{0}; i < SSAO_KERNEL_SIZE; ++i)
    {
        ssaoShader->setVec3("samples[" + std::to_string(i) + "]", m_ssaoKernel[i]);
    }
    ssaoShader->setMat4("projection", projection);
    ssaoShader->setMat4("view", view);
    ssaoShader->setInt("scrWidth", m_width / SSAO_SCALE);
    ssaoShader->setInt("scrHeight", m_height / SSAO_SCALE);
    ssaoShader->setInt("kernelSize", SSAO_KERNEL_SIZE);
    ssaoShader->setFloat("radius", 2.0f);
    ssaoShader->setFloat("bias", 0.025f);

    ssaoShader->setInt("gPositionE", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dfRendererPtr->getPositionEBuffer());
    ssaoShader->setInt("gNormalE", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, dfRendererPtr->getNormalEBuffer());
    ssaoShader->setInt("texNoise", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_noiseTexture);

    renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // blur the result
    glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoFBOBlur);
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT);
    ssaoBlurShader->use();
    ssaoBlurShader->setInt("ssaoTex", 0);
    ssaoBlurShader->setFloat("blurRadius", 3.0f);
    ssaoBlurShader->setFloat("spatialSigma", 2.0f);
    ssaoBlurShader->setFloat("depthSigma", 0.15f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_ssaoColorBuffer);
    renderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
