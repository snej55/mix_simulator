#include "mesh.hpp"
#include <glad/glad.h>

Mesh::Mesh(const std::vector<MeshN::Vertex>& vertices, const std::vector<unsigned int>& indices,
           const std::vector<MeshN::Texture>& textures) : m_vertices{vertices}, m_indices{indices}, m_textures{textures}
{
    setupMesh();
}

void Mesh::render(const Shader* shader) const
{
    shader->use();
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
}

void Mesh::renderPBR(const Shader* pbrShader) const
{
    pbrShader->use();
    std::string textureType;

    for (int i{0}; i < m_textures.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textures[i].id);

        switch (m_textures[i].type)
        {
        case MeshN::TEXTURE_ALBEDO:
            textureType = "albedoMap";
            break;
        case MeshN::TEXTURE_AO:
            textureType = "aoMap";
            break;
        case MeshN::TEXTURE_METALLIC:
            textureType = "metallicMap";
            break;
        case MeshN::TEXTURE_ROUGHNESS:
            textureType = "roughnessMap";
            break;
        case MeshN::TEXTURE_NORMAL:
            textureType = "normalMap";
            break;
        default:
            textureType = "unknown";
            break;
        }

        // don't render unknown texture
        if (textureType == "unknown")
            continue;

        pbrShader->setInt(textureType, i);
    }

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);

    // reset
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

void Mesh::free() const
{
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

void Mesh::setupMesh()
{
    unsigned int meshVAO, meshVBO, meshEBO;
    glGenVertexArrays(1, &meshVAO);
    glGenBuffers(1, &meshVBO);
    glGenBuffers(1, &meshEBO);

    glBindVertexArray(meshVAO);

    glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_vertices.size() * sizeof(MeshN::Vertex)), m_vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_indices.size() * sizeof(unsigned int)),
                 m_indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshN::Vertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshN::Vertex),
                          reinterpret_cast<void*>(offsetof(MeshN::Vertex, normal)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshN::Vertex),
                          reinterpret_cast<void*>(offsetof(MeshN::Vertex, texCoords)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(MeshN::Vertex),
                          reinterpret_cast<void*>(offsetof(MeshN::Vertex, tangent)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(MeshN::Vertex),
                          reinterpret_cast<void*>(offsetof(MeshN::Vertex, biTangent)));
    glEnableVertexAttribArray(4);

    // not really necessary but just in case
    glBindVertexArray(0);

    // set actual VAO, VBO & EBO values
    m_VAO = meshVAO;
    m_VBO = meshVBO;
    m_EBO = meshEBO;

    std::cout << "Loaded mesh: " << m_vertices.size() << " vertices, " << m_indices.size() << " indices" << std::endl;
}
