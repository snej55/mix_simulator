#ifndef MODEL_H
#define MODEL_H

#include "engine_types.hpp"
#include "mesh.hpp"
#include "shader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <map>
#include <string>
#include <vector>

class Model final : public EngineObject
{
public:
    explicit Model(const std::string& name, EngineObject* parent);
    ~Model() override;

    bool loadModel(const std::string& path);

    void render(const Shader* shader) const;
    void renderPBR(const Shader* pbrShader) const;
    
    [[nodiscard]] std::map<std::string, MeshN::BoneInfo>& getBoneInfoMap() {return m_boneInfoMap;}
    [[nodiscard]] int& getBoneCounter() {return m_boneCounter;}

private:
    std::vector<Mesh> m_meshes{};
    std::string directory{};
    std::string m_modelName;

    // loaded mesh textures (to avoid loading the same texture twice)
    std::vector<MeshN::Texture> m_loadedTextures{};
    
    // bones
    std::map<std::string, MeshN::BoneInfo> m_boneInfoMap{};
    int m_boneCounter{0};

    void processNode(const aiNode* node, const aiScene* scene);
    Mesh processMesh(const aiMesh* mesh, const aiScene* scene);

    std::vector<MeshN::Texture> loadMaterialTextures(const aiScene* scene, const aiMaterial* mat, aiTextureType type,
                                                     MeshN::TextureType typeName);
    static unsigned int loadEmbeddedTexture(const aiTexture* texture, bool* success = nullptr,
                                            MeshN::TextureType materialType = MeshN::TEXTURE_NONE);

    static void setDefaultBoneData(MeshN::Vertex& vertex) ;
    static void setVertexBoneData(MeshN::Vertex& vertex, int boneID, float weight);

    void extractBoneWeights(std::vector<MeshN::Vertex>& vertices, const aiMesh* mesh, const aiScene* scene);
};

class ModelManager final : public EngineObject
{
public:
    explicit ModelManager(EngineObject* parent);

    // load new model
    void addModel(const std::string& name, const std::string& path, Arena* arena);

    [[nodiscard]] Model* getModel(const std::string& name) const;

    void renderModel(const Shader* shader, const std::string& name) const;

    [[nodiscard]] bool modelExists(const std::string& name) const;

private:
    std::map<std::string, Model*> m_models{};
};

#endif
