#include "mesh.hpp"
#include <glad/glad.h>
#include "mikktspace.h"

#include <cassert>

Mesh::Mesh(const std::vector<MeshN::Vertex>& vertices, const std::vector<unsigned int>& indices,
           const std::vector<MeshN::Texture>& textures) : m_vertices{vertices}, m_indices{indices}, m_textures{textures}
{
    setupMesh();

    // mikktspace.h
    m_SMT_iface.m_getNumFaces = SMTGetNumFaces;
    m_SMT_iface.m_getNumVerticesOfFace = SMTGetNumVerticesOfFace;
    m_SMT_iface.m_getPosition = SMTGetPosition;
    m_SMT_iface.m_getNormal = SMTGetNormal;
    m_SMT_iface.m_getTexCoord = SMTGetTexCoords;
    m_SMT_iface.m_setTSpaceBasic = SMTSetTSpaceBasic;
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
    // calculate correct tangents
    calcTangents();
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
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(MeshN::Vertex),
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

void Mesh::calcTangents()
{
    m_SMT_context.m_pUserData = this;
    genTangSpaceDefault(&m_SMT_context);
}

int Mesh::SMTGetVertexIndex(const SMikkTSpaceContext* context, const int iFace, const int iVert)
{
    Mesh* mesh {static_cast<Mesh*>(context->m_pUserData)};
    const int faceSize {SMTGetNumVerticesOfFace(context, iFace)};
    const int indicesIndex {iFace * faceSize + iVert};

    return static_cast<int>(mesh->getIndices()[indicesIndex]);
}

int Mesh::SMTGetNumFaces(const SMikkTSpaceContext* context)
{
    Mesh* mesh{static_cast<Mesh*>(context->m_pUserData)};

    const float fSize{static_cast<float>(mesh->getIndices().size()) / 3.f};
    const int iSize{static_cast<int>(mesh->getIndices().size()) / 3};

    assert((fSize - static_cast<float>(iSize)) == 0.f);

    return iSize;
}

int Mesh::SMTGetNumVerticesOfFace(const SMikkTSpaceContext* context, const int iFace)
{
    Mesh* mesh{static_cast<Mesh*>(context->m_pUserData)};
    return 3;
}

void Mesh::SMTGetPosition(const SMikkTSpaceContext* context, float* outPos, const int iFace, const int iVert)
{
    Mesh* mesh{static_cast<Mesh*>(context->m_pUserData)};

    const int index {SMTGetVertexIndex(context, iFace, iVert)};
    const MeshN::Vertex vertex{mesh->getVertices()[index]};

    outPos[0] = vertex.position.x;
    outPos[1] = vertex.position.y;
    outPos[2] = vertex.position.z;
}

void Mesh::SMTGetNormal(const SMikkTSpaceContext* context, float* outNormal, const int iFace, const int iVert)
{
    Mesh* mesh{static_cast<Mesh*>(context->m_pUserData)};

    const int index {SMTGetVertexIndex(context, iFace, iVert)};
    const MeshN::Vertex vertex{mesh->getVertices()[index]};

    outNormal[0] = vertex.normal.x;
    outNormal[1] = vertex.normal.y;
    outNormal[2] = vertex.normal.z;
}

void Mesh::SMTGetTexCoords(const SMikkTSpaceContext* context, float* outUV, const int iFace, const int iVert)
{
    Mesh* mesh{static_cast<Mesh*>(context->m_pUserData)};

    const int index {SMTGetVertexIndex(context, iFace, iVert)};
    const MeshN::Vertex vertex{mesh->getVertices()[index]};

    outUV[0] = vertex.texCoords.x;
    outUV[1] = vertex.texCoords.y;
}

void Mesh::SMTSetTSpaceBasic(const SMikkTSpaceContext* context, const float* tangentU, const float fSign, const int iFace, const int iVert)
{
    Mesh* mesh{static_cast<Mesh*>(context->m_pUserData)};

    const int index{SMTGetVertexIndex(context, iFace, iVert)};
    MeshN::Vertex* vertex{mesh->getVertex(index)};

    vertex->tangent.x = tangentU[0];
    vertex->tangent.y = tangentU[1];
    vertex->tangent.z = tangentU[2];
    vertex->tangent.w = fSign;
}

