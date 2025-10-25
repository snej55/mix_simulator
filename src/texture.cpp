#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

#include "texture.hpp"
#include "util.hpp"
#include "mesh.hpp"

unsigned int TextureN::loadFromFile(const char* path, int* width, int* height, int* numChannels, bool* success,
                                    MeshN::TextureType materialType)
{
    int imageWidth{0};
    int imageHeight{0};
    int imageChannels{0};

    std::cout << path << '\n';
    if (success)
        *success = true;

    // check if texture exists
    if (!Util::fileExists(path))
    {
        Util::beginError();
        std::cout << "TEXTURE::LOAD_FROM_FILE::ERROR: Failed to load texture from path `" << path
                  << "` - texture does not exist";
        Util::endError();
        if (success)
            *success = false;
        return 0;
    }

    // load image
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data{stbi_load(path, &imageWidth, &imageHeight, &imageChannels, 0)};

    // check if image was successfully loaded
    if (!data)
    {
        std::cout << "Failed to load texture: `" << path << "`" << std::endl;
        stbi_image_free(data);
        if (success)
            *success = false;
        return 0;
    }

    // get internal format for tex. data
    GLenum internalFormat{0};
    switch (imageChannels)
    {
    case 1: // grayscale
        internalFormat = GL_RED;
        break;
    case 3:
        internalFormat = GL_RGB;
        break;
    case 4:
        internalFormat = GL_RGBA;
        break;
    default:
        std::cout << "UNKNOWN NUMBER OF CHANNELS: " << imageChannels << std::endl;
        break;
    }

    // texture ID
    unsigned int tex;
    // load opengl texture
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), imageWidth, imageHeight, 0, internalFormat,
                 GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);
    // tex wrap params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // tex filtering params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // check if texture is roughness or metallic map
    // gltf combines roughness and metallic maps, with metallic in b-channel and roughness in g-channel
    // so the texture needs to be swizzled
    if (materialType != MeshN::TEXTURE_NONE)
    {
        switch (materialType)
        {
        case MeshN::TEXTURE_METALLIC:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_BLUE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
            break;
        case MeshN::TEXTURE_ROUGHNESS:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_GREEN);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_GREEN);
            break;
        default:
            break;
        }
    }

    std::cout << "Successfully loaded texture from `" << path << "`\n";

    // free image data
    stbi_image_free(data);

    // update width, height, numChannels
    if (width)
        *width = imageWidth;
    if (height)
        *height = imageHeight;
    if (numChannels)
        *numChannels = imageChannels;

    // return texture id
    return tex;
}

// load hdr irradiance map
unsigned int TextureN::loadHDRMap(const char* path, bool* success)
{
    if (!Util::fileExists(path))
    {
        Util::beginError();
        std::cout << "TEXTUREN::LOAD_HDR_MAP::ERROR: File `" << path << "` does not exist." << std::endl;
        Util::endError();
        *success = false;
        return 0;
    }

    int width{0};
    int height{0};
    int numChannels{0};
    stbi_set_flip_vertically_on_load(true);
    float* data = stbi_loadf(path, &width, &height, &numChannels, 0);

    unsigned int hdrTexture;
    if (data)
    {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        *success = true;
    }
    else
    {
        *success = false;
        hdrTexture = 0;
    }

    stbi_image_free(data);
    return hdrTexture;
}

Texture::Texture(const std::string& name, EngineObject* manager) : EngineObject{("TEXTURE " + name).c_str(), manager} {}

bool Texture::loadFromFile(const char* path)
{
    bool success;
    m_TEX = TextureN::loadFromFile(path, &m_width, &m_height, &m_numChannels, &success);
    return success;
}

// activate gl texture
void Texture::activate(const int slot) const
{
    // select texture slot
    glActiveTexture(GL_TEXTURE0 + slot);
    // activate texture
    glBindTexture(GL_TEXTURE_2D, m_TEX);
}

// ------- Texture Manager ------- //
TextureManager::TextureManager(EngineObject* parent) : EngineObject{"TextureManager", parent} {}

// generate vertex buffers and stuff
void TextureManager::generateBuffers()
{
    constexpr float TexRectVertices[]{
        1.0f, 0.0f,  0.0f, 1.0f, 1.0f, // top right
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom right
        0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
        0.0f, 0.0f,  0.0f, 0.0f, 1.0f // top left
    };

    constexpr unsigned int TexRectIndices[]{
        0, 1, 3, // first Triangle
        1, 2, 3 // second Triangle
    };

    // generate VAO and buffers
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    // bind vertex array
    glBindVertexArray(m_VAO);

    // buffer vertex data
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TexRectVertices), TexRectVertices, GL_STATIC_DRAW);

    // buffer vertex indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(TexRectIndices), TexRectIndices, GL_STATIC_DRAW);

    // vertex coordinates
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // can safely unbind (NOTE: Don't unbind element array buffer)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// load new texture
void TextureManager::addTexture(const char* path, const char* name, Arena* arena)
{
    // create new texture and add to arena
    Texture* texture{new Texture{name, this}};
    arena->addObject(texture);

    // load texture
    if (!texture->loadFromFile(path))
    {
        Util::beginError();
        std::cout << "TEXTURE_MANAGER::ADD_TEXTURE::ERROR: Failed to add texture `" << name << "`!";
        Util::endError();
    }
    else
    {
        m_textures.insert(std::pair{std::string{name}, texture});
    }
}

Texture* TextureManager::getTexture(const std::string& name) const
{
    if (textureExists(name))
    {
        return m_textures.find(name)->second;
    }

    Util::beginError();
    std::cout << "TEXTURE_MANAGER::GET_TEXTURE::ERROR: Texture `" << name << "' does not exist!";
    Util::endError();
    return nullptr;
}

void TextureManager::activateTexture(const std::string& name, int slot) const
{
    if (textureExists(name))
    {
        getTexture(name)->activate(slot);
    }
    else
    {
        Util::beginError();
        std::cout << "TEXTURE_MANAGER::ACTIVATE_TEXTURE::ERROR: Texture `" << name << "' does not exist!";
        Util::endError();
    }
}

bool TextureManager::textureExists(const std::string& name) const { return m_textures.find(name) != m_textures.end(); }
