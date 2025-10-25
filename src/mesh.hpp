#ifndef MESH_H
#define MESH_H

#include "shader.hpp"

#include <vector>

#include <glm/glm.hpp>

namespace MeshN
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
        glm::vec3 tangent; // TBN matrix
        glm::vec3 biTangent; // "" ""
    };

    enum TextureType
    {
        TEXTURE_ALBEDO = 0,
        TEXTURE_AO = 1,
        TEXTURE_METALLIC = 2,
        TEXTURE_ROUGHNESS = 3,
        TEXTURE_NORMAL = 4,
        TEXTURE_NONE = 5,
    };

    struct Texture
    {
        unsigned int id;
        TextureType type;
        std::string path;
        bool embedded;
    };
} // namespace MeshN

class Mesh
{
public:
    Mesh(const std::vector<MeshN::Vertex>& vertices, const std::vector<unsigned int>& indices,
         const std::vector<MeshN::Texture>& textures);

    void render(const Shader* shader) const;
    void renderPBR(const Shader* pbrShader) const;

    void free() const;

    [[nodiscard]] const std::vector<MeshN::Vertex>& getVertices() const { return m_vertices; }
    [[nodiscard]] const std::vector<unsigned int>& getIndices() const { return m_indices; }

private:
    std::vector<MeshN::Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<MeshN::Texture> m_textures;

    unsigned int m_VAO{};
    unsigned int m_VBO{};
    unsigned int m_EBO{};

    void setupMesh();
};

#endif // MESH_H
