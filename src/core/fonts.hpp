#ifndef FONTS_H
#define FONTS_H

#include <glad/glad.h>
#include <ft2build.h>
#include FT_FREETYPE_H

// glm libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <map>

#include "shader.hpp"
#include "engine_types.hpp"

struct Character
{
    unsigned int textureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

class FontManager : public EngineObject
{
public:
    explicit FontManager(EngineObject* parent);
    ~FontManager();

    bool init(const std::string& font, int height);
    void free();

    // render text
    void renderText(const Shader* shader, std::string text, float x, float y, float scale, const glm::vec3&& color);

    // update projection matrix with framebuffer dimensions on resize
    void updateProjection(float width, float height);

    // getters & setters for font face and library
    [[nodiscard]] const FT_Face& getFace() const { return m_face; }
    [[nodiscard]] const FT_Library& getLibrary() const { return m_FT; }

    // font path and loaded flag
    [[nodiscard]] std::string_view getFontPath() const { return m_fontPath; }
    [[nodiscard]] bool getLoaded() const { return m_loaded; }

    // vertex array object & vertex buffer object
    [[nodiscard]] unsigned int getVAO() const { return m_VAO; }
    [[nodiscard]] unsigned int getVBO() const { return m_VBO; }

private:
    FT_Face m_face{}; // font face
    FT_Library m_FT{}; // freetype library

    std::string m_fontPath{""}; // path of the current font
    bool m_loaded{false}; // whether a font has been loaded or not

    std::map<char, Character> m_characters{};

    unsigned int m_VAO{0};
    unsigned int m_VBO{0};

    glm::mat4 m_projection{};
};

#endif
