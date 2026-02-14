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
    Model(const Model& other);
    Model& operator=(const Model& other) = delete;
    ~Model() override;

    bool loadModel(const std::string& path);

    void render(const Shader* shader) const;
    void renderDepth() const;
    void renderPBR(const Shader* pbrShader) const;
    // forward render the model
    void renderForward(const Shader* pbrShader, const glm::vec3& cameraPos, const glm::mat4& model) const;
    // deferred render the model
    void renderDeferred(const Shader* dfShader, const glm::mat4& model) const;

    // animation loading & playback
    void updateAnimation(float deltaTime) const;
    void loadAnimation();
    void freeAnimation();

    // getters
    [[nodiscard]] std::string_view getDirectory() const { return m_directory; }
    [[nodiscard]] std::string_view getPath() const { return m_path; }

    [[nodiscard]] const std::vector<Mesh*>& getOpaqueMeshes() const { return m_opaqueMeshes; }
    [[nodiscard]] const std::vector<Mesh*>& getTransparentMeshes() const { return m_transparentMeshes; }

    [[nodiscard]] std::map<std::string, MeshN::BoneInfo>& getBoneInfoMap() { return m_boneInfoMap; }
    [[nodiscard]] int& getBoneCounter() { return m_boneCounter; }

    [[nodiscard]] bool isAnimated() const { return m_animated; }
    [[nodiscard]] void* getAnimation() const { return m_animation; }
    [[nodiscard]] void* getAnimator() const { return m_animator; }
    [[nodiscard]] const std::vector<glm::mat4>& getAnimationTransforms() const;

private:
    std::string m_modelName;

    std::vector<Mesh> m_meshes{};
    std::string m_directory{};
    std::string m_path{};

    // loaded mesh textures (to avoid loading the same texture twice)
    std::vector<MeshN::Texture> m_loadedTextures{};

    // opaque meshes (rendered deferred)
    std::vector<Mesh*> m_opaqueMeshes{};
    // transparent meshes (require blending so rendered forward)
    std::vector<Mesh*> m_transparentMeshes{};

    // bones
    std::map<std::string, MeshN::BoneInfo> m_boneInfoMap{};
    int m_boneCounter{0};

    bool m_animated{false};
    void* m_animation{nullptr};
    void* m_animator{nullptr};

    void processNode(const aiNode* node, const aiScene* scene);
    Mesh processMesh(const aiMesh* mesh, const aiScene* scene);

    std::vector<MeshN::Texture> loadMaterialTextures(const aiScene* scene, const aiMaterial* mat, aiTextureType type,
                                                     MeshN::TextureType typeName);
    static unsigned int loadEmbeddedTexture(const aiTexture* texture, bool* success = nullptr,
                                            MeshN::TextureType materialType = MeshN::TEXTURE_NONE);

    static void setDefaultBoneData(MeshN::Vertex& vertex);
    static void setVertexBoneData(MeshN::Vertex& vertex, int boneID, float weight);
    static bool checkVertexWeights(const MeshN::Vertex& vertex);

    void extractBoneWeights(std::vector<MeshN::Vertex>& vertices, const aiMesh* mesh);

    void handleTransparentTextures(const aiScene* scene);

    // pbr material parameters
    static void loadEmissiveFactor(const aiMaterial* mat, glm::vec3& emissiveFactor, float& emissiveIntensity);
    static void loadBaseColor(const aiMaterial* mat, glm::vec4& baseColor);
    static void loadMetallicFactor(const aiMaterial* mat, float& metallicFactor);
    static void loadRoughnessFactor(const aiMaterial* mat, float& roughnessFactor);
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
    [[nodiscard]] Model* getModelByPath(const std::string& paths) const;

private:
    std::map<std::string, Model*> m_models{};
};

#endif
