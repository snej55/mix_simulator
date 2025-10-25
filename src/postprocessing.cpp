#include "postprocessing.hpp"

#include "engine.hpp"
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
