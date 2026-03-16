#include <iostream>

#include "engine_types.hpp"
#include "freetype/ftglyph.h"
#include "fonts.hpp"

FontManager::FontManager(EngineObject* parent) : EngineObject("fontManager", parent) {}

FontManager::~FontManager() { free(); }

bool FontManager::init(const std::string& path, const int height)
{
    m_loaded = false;
    // initialize freetype2 library
    if (FT_Init_FreeType(&m_FT))
    {
        std::cout << "ERROR::FONT_MANAGER: Could not init FreeType library" << std::endl;
        return false;
    }

    // load font
    if (FT_New_Face(m_FT, path.c_str(), 0, &m_face))
    {
        std::cout << "ERROR::FONT_MANAGER: Failed to load font at `" << path << "`" << std::endl;
        return false;
    }

    FT_Stroker stroker;
    FT_Stroker_New(m_FT, &stroker);
    FT_Stroker_Set(stroker, 64 * 2, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

    // set font size
    FT_Set_Pixel_Sizes(m_face, 0, height);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // first 128 ascii characters
    for (unsigned char c{0}; c < 128; c++)
    {
        // load character glyph
        if (FT_Load_Char(m_face, c, FT_LOAD_DEFAULT)) // NOTE: FT_LOAD_RENDER loads bitmap immediately and discards
                                                      // vector data (which is needed for strokes)
        {
            std::cout << "ERROR::FONT_MANAGER: Failed to load glyph" << std::endl;
            continue; // go to next character
        }

        FT_Glyph outlineGlyph;
        FT_Get_Glyph(m_face->glyph, &outlineGlyph);
        FT_Glyph_Stroke(&outlineGlyph, stroker, 1);
        FT_Glyph_To_Bitmap(&outlineGlyph, FT_RENDER_MODE_NORMAL, 0, 1);
        FT_BitmapGlyph outline{reinterpret_cast<FT_BitmapGlyph>(outlineGlyph)};

        unsigned int outlineTex;
        glGenTextures(1, &outlineTex);
        glBindTexture(GL_TEXTURE_2D, outlineTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, outline->bitmap.width, outline->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                     outline->bitmap.buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL);

        // generate the texture
        unsigned int tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows, 0, GL_RED,
                     GL_UNSIGNED_BYTE, m_face->glyph->bitmap.buffer);
        // texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // store the character in character map
        Character character{tex,
                            outlineTex,
                            glm::ivec2{m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows},
                            glm::ivec2{m_face->glyph->bitmap_left, m_face->glyph->bitmap_top},
                            glm::ivec2{outline->bitmap.width, outline->bitmap.rows},
                            glm::ivec2{outline->left, outline->top},
                            static_cast<unsigned int>(m_face->glyph->advance.x)};
        // insert character into map
        m_characters.insert(std::pair<char, Character>{c, character});
        FT_Done_Glyph(outlineGlyph);
    }

    FT_Stroker_Done(stroker);

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
    std::cout << "Successfully loaded font from `" << path << "`\n";
    return true;
}

void FontManager::free()
{
    if (m_loaded)
    {
        FT_Done_Face(m_face);
        FT_Done_FreeType(m_FT);
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
    }
}

void FontManager::renderText(const Shader* shader, const std::string text, float xcoord, float y, const float scale,
                             const glm::vec3&& color)
{
    // use shader
    shader->use();
    shader->setMat4("projection", m_projection);
    shader->setInt("text", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_VAO);

    // go through all the characters
    std::string::const_iterator chr;
    float x{xcoord};
    shader->setVec3("textColor", {0.1f, 0.1f, 0.1f});
    Character c{};
    for (chr = {text.begin()}; chr != text.end(); ++chr)
    {
        c = m_characters[*chr];

        const float xpos{x + c.outBearing.x * scale};
        const float ypos{y - (c.outSize.y - c.outBearing.y) * scale};

        const float w{c.outSize.x * scale};
        const float h{c.outSize.y * scale};
        // update vbo for each character
        float vertices[6][4] = {// first triangle
                                {xpos, ypos + h, 0.0f, 0.0f},
                                {xpos, ypos, 0.0f, 1.0f},
                                {xpos + w, ypos, 1.0f, 1.0f},
                                // second triangle
                                {xpos, ypos + h, 0.0f, 0.0f},
                                {xpos + w, ypos, 1.0f, 1.0f},
                                {xpos + w, ypos + h, 1.0f, 0.0f}};
        // render glyph texture on quad
        glBindTexture(GL_TEXTURE_2D, c.outlineID);
        // update VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // advance cursor for next glyph
        x += (c.advance >> 6) * scale; // black magic (bitshift by 6 gives value in pixels (2^6 = 64))
    }
    x = xcoord;
    shader->setVec3("textColor", color);
    for (chr = {text.begin()}; chr != text.end(); ++chr)
    {
        Character c{m_characters[*chr]};

        const float xpos{x + c.bearing.x * scale};
        const float ypos{y - (c.size.y - c.bearing.y) * scale};

        const float w{c.size.x * scale};
        const float h{c.size.y * scale};
        // update vbo for each character
        float vertices[6][4] = {// first triangle
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
        x += (c.advance >> 6) * scale; // black magic (bitshift by 6 gives value in pixels (2^6 = 64))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

float FontManager::getTextWidth(const std::string& text, const float scale)
{
    float x{0.0f};
    std::string::const_iterator chr;
    Character c{};
    for (chr = {text.begin()}; chr != text.end(); ++chr)
    {
        c = m_characters[*chr]; // NOTE: [] operator is non const

        x += (c.advance >> 6) * scale;
    }
    x += c.bearing.x * scale;
    return x;
}

void FontManager::updateProjection(const float width, const float height)
{
    // update projection matrix with new framebuffer dimensions
    m_projection = glm::ortho(0.0f, width, 0.0f, height);
}
