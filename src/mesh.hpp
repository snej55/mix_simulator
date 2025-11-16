#ifndef MESH_H
#define MESH_H

#include "shader.hpp"

#include <vector>

#include <glm/glm.hpp>
#include <mikktspace.h>

#define MAX_BONE_INFLUENCE 4

namespace MeshN
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
        glm::vec4 tangent; // TBN matrix
        glm::vec3 biTangent; // "" ""
        int boneIDs[MAX_BONE_INFLUENCE];
        float weights[MAX_BONE_INFLUENCE];
    };

    enum TextureType
    {
        TEXTURE_ALBEDO = 0,
        TEXTURE_AO = 1,
        TEXTURE_METALLIC = 2,
        TEXTURE_ROUGHNESS = 3,
        TEXTURE_NORMAL = 4,
        TEXTURE_EMISSIVE = 5,
        TEXTURE_NONE = 6,
    };

    enum BlendMode
    {
        BLEND_OPAQUE = 0,
        BLEND_TRANSPARENT = 1,
        BLEND_MASK = 2,
        BLEND_NONE = 3,
    };

    struct Texture
    {
        unsigned int id;
        TextureType type;
        std::string path;
        bool embedded;
    };

    struct BoneInfo
    {
        int id;
        glm::mat4 offset; // from model space to bone space
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

    void calcTangents();

    [[nodiscard]] const std::vector<MeshN::Vertex>& getVertices() const { return m_vertices; }
    [[nodiscard]] MeshN::Vertex* getVertex(const int index) { return &m_vertices[index]; }
    [[nodiscard]] const std::vector<unsigned int>& getIndices() const { return m_indices; }

    void setBlendMode(const MeshN::BlendMode blendMode) { m_blendMode = blendMode; }
    [[nodiscard]] MeshN::BlendMode getBlendMode() const { return m_blendMode; }

    // calculate AABB midpoint (world center)
    void calculateMidpoint(const glm::mat4& transform);
    void updateMidpoint(const glm::mat4& transform); // update model matrix
    [[nodiscard]] glm::vec3 getMidpoint() const {return m_midPoint;}
    [[nodiscard]] const glm::mat4& getModelTransform() const {return m_modelMat;}

private:
    std::vector<MeshN::Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<MeshN::Texture> m_textures;

    unsigned int m_VAO{};
    unsigned int m_VBO{};
    unsigned int m_EBO{};

    // for depth sorting
    MeshN::BlendMode m_blendMode{MeshN::BLEND_OPAQUE};
    glm::vec3 m_midPoint{};
    glm::vec3 m_localPos{};
    glm::mat4 m_modelMat{};

    SMikkTSpaceContext m_SMT_context{};
    SMikkTSpaceInterface m_SMT_iface{};

    void setupMesh();

    // SMikkT callbacks
    static int SMTGetVertexIndex(const SMikkTSpaceContext* context, int iFace, int iVert);

    static int SMTGetNumFaces(const SMikkTSpaceContext* context);
    static int SMTGetNumVerticesOfFace(const SMikkTSpaceContext* context, int iFace);
    static void SMTGetPosition(const SMikkTSpaceContext* context, float outPos[], int iFace, int iVert);

    static void SMTGetNormal(const SMikkTSpaceContext* context, float outNormal[], int iFace, int iVert);

    static void SMTGetTexCoords(const SMikkTSpaceContext* context, float outUV[], int iFace, int iVert);

    static void SMTSetTSpaceBasic(const SMikkTSpaceContext* context, const float tangentU[], float fSign,
                                  int iFace, int iVert);
};

#endif // MESH_H
