#include "postprocessing.hpp"

#include "engine.hpp"
#include "engine_types.hpp"
#include "util.hpp"

PostProcessor::PostProcessor(EngineObject* parent) : EngineObject{"PostProcessor", parent} {}

PostProcessor::~PostProcessor() { free(); }

// free framebuffer
void PostProcessor::free()
{
    glDeleteTextures(1, &m_TEX);
    glDeleteRenderbuffers(1, &m_RBO);
    glDeleteFramebuffers(1, &m_FBO);
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

    // create quad
    generateQuad();
}

void PostProcessor::render(const Shader* screenShader) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    // clear buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    screenShader->use();
    screenShader->setInt("screenTexture", 0);

    glBindVertexArray(m_VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TEX);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

void PostProcessor::generate(const int width, const int height)
{
    free();
    init(width, height);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // attach rbo to framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
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

void PostProcessor::disable() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

BloomFBO::BloomFBO(EngineObject* parent)
 : EngineObject{"BloomFBO", parent}
{
}

BloomFBO::~BloomFBO()
{
    free();
}

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

    glm::vec2 mipSize {static_cast<float>(width), static_cast<float>(height)};
    glm::ivec2 mipIntSize {static_cast<int>(width), static_cast<int>(height)};

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
	PostProcessingN::BloomMip mip;
	
	mipSize *= 0.5f;
	mipIntSize /= 2;
	mip.size = mipSize;
	mip.intSize = mipIntSize;

	glGenTextures(1, &mip.texture);
	glBindTexture(GL_TEXTURE_2D, mip.texture);
	// MOTE: hdr color format
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, static_cast<int>(mipSize.x), static_cast<int>(mipSize.y), 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_mipChain.emplace_back(mip);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_mipChain[0].texture, 0);

    // setup attachments
    unsigned int attachments[1] {GL_COLOR_ATTACHMENT0};
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

void BloomFBO::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
}

BloomRenderer::BloomRenderer(EngineObject* parent)
 : EngineObject{"BloomRenderer", parent}, m_FBO{this}
{
}

BloomRenderer::~BloomRenderer()
{
    free();
}

void BloomRenderer::free()
{
    if (!m_init)
	return;

    m_FBO.free();
    m_init = false;
}

bool BloomRenderer::init(const unsigned int width, const unsigned int height, void* engine)
{
    if (m_init)
	return true;

    m_srcViewportSize = glm::ivec2{width, height};
    m_srcViewportSizeF = glm::vec2{static_cast<float>(width), static_cast<float>(height)};
    
    constexpr unsigned int numMips {5};
    if (!m_FBO.init(width, height, numMips))
    {
	Util::beginError();
	std::cout << "BLOOM_RENDERER::INIT::ERROR: Failed to create Bloom FBO!" << std::endl;
	Util::endError();
	return false;
    }

    const Engine* enginePtr {static_cast<Engine*>(engine)};
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

    m_init = true;
    return true;
}
