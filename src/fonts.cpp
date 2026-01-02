//
// Created by jenskromdijk on 06/10/2025.
//

#include <glad/glad.h>
#include <iostream>

#include "fonts.hpp"

FontRenderer::FontRenderer(EngineObject* parent) : EngineObject{"FontRenderer", parent} {}

FontRenderer::~FontRenderer() { free(); }

bool FontRenderer::init(const std::string& fontPath, const int height)
{
    m_loaded = false;
    // initialize freetype2 library
    if (FT_Init_FreeType(&m_FT))
    {
        std::cout << "ERROR::FONT_MANAGER: Could not init FreeType library" << std::endl;
        return true;
    }

    // load font
    if (FT_New_Face(m_FT, fontPath.c_str(), 0, &m_face))
    {
        std::cout << "ERROR::FONT_MANAGER: Failed to load font at `" << fontPath << "`" << std::endl;
        return true;
    }

    // set font size
    FT_Set_Pixel_Sizes(m_face, 0, height);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // first 128 ascii characters
    for (unsigned char c{0}; c < 128; c++)
    {
        // load character glyph
        if (FT_Load_Char(m_face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FONT_MANAGER: Failed to load glyph" << std::endl;
            continue; // go to next character
        }

        // generate the texture
        unsigned int tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<GLsizei>(m_face->glyph->bitmap.width),
                     static_cast<GLsizei>(m_face->glyph->bitmap.rows), 0, GL_RED, GL_UNSIGNED_BYTE,
                     m_face->glyph->bitmap.buffer);
        // texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // store the character in character map
        Character character{tex, glm::ivec2{m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows},
                            glm::ivec2{m_face->glyph->bitmap_left, m_face->glyph->bitmap_top},
                            static_cast<unsigned int>(m_face->glyph->advance.x)};
        // insert character into map
        m_characters.insert(std::pair<char, Character>{c, character});
    }

    // generate vertex arrays & vbo
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    // enough memory for rendering characters
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // all good
    m_loaded = true;
    std::cout << "Successfully loaded font from `" << fontPath << "`\n";
    return false;
}

void FontRenderer::free() const
{
    if (m_loaded)
    {
        FT_Done_Face(m_face);
        FT_Done_FreeType(m_FT);
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
    }
}

void FontRenderer::renderText(const Shader& shader, const std::string& text, float x, const float y, const float scale,
                              const glm::vec3&& color)
{
    // correct blending function
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // use shader
    shader.use();
    shader.setVec3("textColor", color);
    shader.setMat4("projection", m_projection);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_VAO);

    // go through all the characters
    std::string::const_iterator chr;
    for (chr = {text.begin()}; chr != text.end(); ++chr)
    {
        Character c{m_characters[*chr]};

        const float xpos{x + static_cast<float>(c.bearing.x) * scale};
        const float ypos{y - static_cast<float>(c.size.y - c.bearing.y) * scale};

        const float w{static_cast<float>(c.size.x) * scale};
        const float h{static_cast<float>(c.size.y) * scale};
        // update vbo for each character
        const float vertices[6][4] = {// first triangle
                                      {xpos, ypos + h, 0.0f, 0.0f},
                                      {xpos, ypos, 0.0f, 1.0f},
                                      {xpos + w, ypos, 1.0f, 1.0f},
                                      // second triangle
                                      {xpos, ypos + h, 0.0f, 0.0f},
                                      {xpos + w, ypos, 1.0f, 1.0f},
                                      {xpos + w, ypos + h, 1.0f, 0.0f}};
        // render glyph texture on quad
        glBindTexture(GL_TEXTURE_2D, c.textureID);
        // update VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // advance cursor for next glyph
        x += static_cast<float>(c.advance >> 6) * scale; // black magic (bitshift by 6 gives value in pixels (2^6 = 64))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void FontRenderer::updateProjection(const float width, const float height)
{
    // update projection matrix with new framebuffer dimensions
    m_projection = glm::ortho(0.0f, width, 0.0f, height);
}
