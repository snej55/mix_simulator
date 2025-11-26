//
// Created by Jens Kromdijk on 23/06/25.
//

#include <STB/stb_image.h>
#include <assimp/postprocess.h>
#include <cstddef>
#include <glad/glad.h>
#include <limits>
#include <mikktspace.h>
#include <assimp/material.h>
#include <assimp/GltfMaterial.h>

#include "mesh.hpp"
#include "model.hpp"
#include "texture.hpp"
#include "util.hpp"

#include <sstream>
#include <string>

Model::Model(const std::string& name, EngineObject* parent) :
    EngineObject{("MODEL " + name).c_str(), parent}, m_modelName{name}
{
}

Model::~Model()
{
    for (std::size_t i{0}; i < m_meshes.size(); ++i)
    {
        m_meshes[i].free();
    }
}

void Model::render(const Shader* shader) const
{
    for (std::size_t i{0}; i < m_meshes.size(); ++i)
    {
        m_meshes[i].render(shader);
    }
}

void Model::renderPBR(const Shader* pbrShader) const
{
    for (std::size_t i{0}; i < m_meshes.size(); ++i)
    {
        m_meshes[i].renderPBR(pbrShader);
    }
}

// forward render the model
void Model::renderForward(const Shader* pbrShader, const glm::vec3& cameraPos, const glm::mat4& model) const
{
    std::map<float, Mesh*> sortedMeshes{};
    for (Mesh* mesh : m_transparentMeshes)
    {
        mesh->updateMidpoint(model);
        const float distance {glm::length(cameraPos - mesh->getMidpoint())};
        sortedMeshes[distance] = mesh;
    }

    // ----- render the opaque meshes first ----- //
    for (const Mesh* mesh : m_opaqueMeshes)
    {
        mesh->renderPBR(pbrShader);
    }

    // ----- render the transparent objects ----- //
    // enable the correct blending equation (messed up by bloom)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_FALSE);

    for (std::map<float, Mesh*>::reverse_iterator it{sortedMeshes.rbegin()}; it != sortedMeshes.rend(); ++it)
    {
        it->second->renderPBR(pbrShader);
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void Model::renderHybrid(const Shader* dfShader, const Shader* fdShader, const glm::vec3& cameraPos,
    const glm::mat4& model) const
{
    // sort transparent meshes (to be rendered forward)
    std::map<float, Mesh*> sortedMeshes{};
    for (Mesh* mesh : m_transparentMeshes)
    {
        mesh->updateMidpoint(model);
        const float distance {glm::length(cameraPos - mesh->getMidpoint())};
        sortedMeshes[distance] = mesh;
    }

    // render opaque meshes deferred
    for (const Mesh* mesh : m_opaqueMeshes)
    {
        mesh->renderPBR(dfShader);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_FALSE);

    for (std::map<float, Mesh*>::reverse_iterator it{sortedMeshes.rbegin()}; it != sortedMeshes.rend(); ++it)
    {
        it->second->renderPBR(fdShader);
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

bool Model::loadModel(const std::string& path)
{
    // check if model already exists
    if (!Util::fileExists(path))
    {
        Util::beginError();
        std::cout << "MODEL::LOAD_MODEL::ERROR: Failed to load model from `" << path << "` - file does not exist!";
        Util::endError();
        return false;
    }

    Assimp::Importer importer;

    const aiScene* scene{importer.ReadFile(
        path,
        aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes)};

    // error handling
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        // if it isn't zero
        Util::beginError();
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString();
        Util::endError();
        return false;
    }

    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
    handleTransparentTextures(scene);

    // overkill log
    int numVertices{};
    for (std::size_t i{0}; i < m_meshes.size(); ++i)
    {
        numVertices += static_cast<int>(m_meshes[i].getVertices().size());
        // we're looping anyway so might as well add it here (sort meshes into opaque and blended)
        if (m_meshes[i].getBlendMode() == MeshN::BLEND_TRANSPARENT)
        {
            m_transparentMeshes.emplace_back(&m_meshes[i]);
        }
        else
        {
            m_opaqueMeshes.emplace_back(&m_meshes[i]);
        }
    }

    // just some useful info :)
    unsigned long vertSize{sizeof(MeshN::Vertex) * numVertices};
    std::stringstream ss{};
    if (vertSize > 1000 * 1000)
    {
        vertSize = vertSize / 1000 / 1000;
        ss << vertSize << " MB";
    }
    else if (vertSize > 1000)
    {
        vertSize = vertSize / 1000;
        ss << vertSize << " KB";
    }
    else
    {
        ss << vertSize << " B";
    }

    const std::string size = ss.str();
    std::cout << "Loaded model at `" << path << "`, " << numVertices << " vertices (" << size << ")" << std::endl;

    return true;
}

void Model::processNode(const aiNode* node, const aiScene* scene)
{
    for (std::size_t i{0}; i < node->mNumMeshes; ++i)
    {
        // node->mMeshes is a list of indices for scene->mMeshes
        const aiMesh* mesh{scene->mMeshes[node->mMeshes[i]]};

        m_meshes.emplace_back(processMesh(mesh, scene));
    }

    // repeat recursively for all children
    for (std::size_t i{0}; i < node->mNumChildren; ++i)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(const aiMesh* mesh, const aiScene* scene)
{
    std::vector<MeshN::Vertex> vertices{};
    std::vector<unsigned int> indices{};
    std::vector<MeshN::Texture> textures{};

    for (std::size_t i{0}; i < mesh->mNumVertices; ++i)
    {
        MeshN::Vertex vertex{};
        // get vertex positions
        const glm::vec3 pos{mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        // same for normals
        const glm::vec3 normal{mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
        // texture coordinates if mesh has them
        if (mesh->mTextureCoords[0])
        {
            const glm::vec2 texCoords{mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
            vertex.texCoords = texCoords;
        }
        else
        {
            vertex.texCoords = glm::vec2{0.0f, 0.0f};
        }

        // calculate tangent and bitangent for normal mapping
        const glm::vec4 tangent{mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, 0.0f};

        const glm::vec3 biTangent{mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};

        vertex.position = pos;
        vertex.normal = normal;
        vertex.tangent = tangent;
        vertex.biTangent = biTangent;
        vertices.push_back(vertex);
    }

    // indices
    for (unsigned int i{0}; i < mesh->mNumFaces; ++i)
    {
        const aiFace& face{mesh->mFaces[i]};
        // each face usually has like 3 indices or something
        for (unsigned int j{0}; j < face.mNumIndices; ++j)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // load bones
    for (MeshN::Vertex& vertex : vertices)
    {
        setDefaultBoneData(vertex);
    }
    extractBoneWeights(vertices, mesh);

    // materials
    aiMaterial* material{scene->mMaterials[mesh->mMaterialIndex]};
    MeshN::Material meshMaterial{};

    // use custom glTF Material Output node in blender for ambient occlusion texture
    std::vector<MeshN::Texture> aoMaps{
        loadMaterialTextures(scene, material, aiTextureType_LIGHTMAP, MeshN::TEXTURE_AO)};
    if (aoMaps.empty())
        meshMaterial.useAOTex = false;
    else
        textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());

    // albedo texture
    std::vector<MeshN::Texture> albedoMaps{
        loadMaterialTextures(scene, material, aiTextureType_BASE_COLOR, MeshN::TEXTURE_ALBEDO)};
    if (albedoMaps.empty())
    {
        loadBaseColor(material, meshMaterial.albedo);
        meshMaterial.useAlbedoTex = false;
    }
    else
    {
        textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());
    }

    // metallic texture (b-channel of metallic-roughness texture)
    std::vector<MeshN::Texture> metallicMaps{
        loadMaterialTextures(scene, material, aiTextureType_METALNESS, MeshN::TEXTURE_METALLIC)};
    if (metallicMaps.empty())
    {
        loadMetallicFactor(material, meshMaterial.metallicFactor);
        meshMaterial.useMetallicTex = false;
    }
    else
    {
        textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());
    }
    // roughness texture (g-channel)
    std::vector<MeshN::Texture> roughnessMaps{
        loadMaterialTextures(scene, material, aiTextureType_GLTF_METALLIC_ROUGHNESS, MeshN::TEXTURE_ROUGHNESS)};
    if (roughnessMaps.empty())
    {
        loadRoughnessFactor(material, meshMaterial.roughnessFactor);
        meshMaterial.useRoughnessTex = false;
    }
    else
    {
        textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
    }
    // normal map texture
    std::vector<MeshN::Texture> normalMaps{
        loadMaterialTextures(scene, material, aiTextureType_NORMALS, MeshN::TEXTURE_NORMAL)};
    if (normalMaps.empty())
        meshMaterial.useNormalTex = false;
    else
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    // emission factor
    std::vector<MeshN::Texture> emissiveMaps{
        loadMaterialTextures(scene, material, aiTextureType_EMISSIVE, MeshN::TEXTURE_EMISSIVE)};
    if (emissiveMaps.empty())
    {
        glm::vec3 emissiveFactor{0.0f, 0.0f, 0.0f};
        float emissiveIntensity{1.0f};
        loadEmissiveFactor(material, emissiveFactor, emissiveIntensity);
        meshMaterial.emissiveFactor = emissiveFactor;
        meshMaterial.emissiveIntensity = emissiveIntensity;
        meshMaterial.useEmissiveTex = false;
    }
    else
    {
        textures.insert(textures.end(), emissiveMaps.begin(), emissiveMaps.end());
    }

    return Mesh{vertices, indices, textures, meshMaterial};
}

std::vector<MeshN::Texture> Model::loadMaterialTextures(const aiScene* scene, const aiMaterial* mat,
                                                        const aiTextureType type, const MeshN::TextureType typeName)
{
    std::vector<MeshN::Texture> textures{};
    for (unsigned int i{0}; i < mat->GetTextureCount(type); ++i)
    {
        aiString str;
        mat->Get(AI_MATKEY_TEXTURE(type, i), str);
        bool skip{false};

        // check if we haven't already loaded this texture
        for (unsigned int j{0}; j < m_loadedTextures.size(); ++j)
        {
            // compare
            if (std::strcmp(m_loadedTextures[j].path.c_str(), str.C_Str()) == 0)
            {
                // we found something with the same path
                // check if texture type is the same, and it isn't embedded (path is *1 or smth)
                if (m_loadedTextures[j].type == typeName && !m_loadedTextures[j].embedded)
                {
                    // push back THAT texture instead
                    textures.push_back(m_loadedTextures[j]);
                    skip = true;
                    break;
                }
            }
        }

        // jump to next iteration if we already loaded this texture
        if (skip)
            continue;

        unsigned int texID;
        bool success;

        // check if texture is embedded in scene or separate
        if (const aiTexture* texPtr = scene->GetEmbeddedTexture(str.C_Str()))
        {
            // if texPtr isn't nullptr, texture can be read from memory
            texID = loadEmbeddedTexture(texPtr, &success, typeName);
        }
        else
        {
            // get texture path
            std::string filename{directory + '/' + str.C_Str()};
            // load texture id
            texID = TextureN::loadFromFile(filename.c_str(), nullptr, nullptr, nullptr, &success, typeName);
        }

        if (!success) // check if texture was loaded successfully (don't add bad texture)
        {
            continue;
        }

        // create texture object
        MeshN::Texture texture{texID, // texture id
                               typeName, // MeshN::TextureType
                               str.C_Str(), // texture path
                               false};

        m_loadedTextures.push_back(texture);
        textures.push_back(texture);
    }

    return textures;
}

// load texture embedded in scene
unsigned int Model::loadEmbeddedTexture(const aiTexture* texture, bool* success, const MeshN::TextureType materialType)
{
    int imageWidth{0};
    int imageHeight{0};
    int imageChannels{0};
    unsigned char* data{nullptr};

    if (success)
        *success = true;

    // load texture data from memory
    // stbi_set_flip_vertically_on_load(true);
    data = stbi_load_from_memory(
        // texture data
        reinterpret_cast<unsigned char*>(texture->pcData),
        // buffer length
        static_cast<int>(texture->mWidth * (texture->mHeight == 0 ? 1 : texture->mHeight)), &imageWidth, &imageHeight,
        &imageChannels, 0);

    // check success
    if (!data)
    {
        std::cout << "MODEL::LOAD_EMBEDDED_TEXTURE::ERROR: Failed to load texture from memory!\n";
        stbi_image_free(data);
        if (success)
            *success = false;
        return 0;
    }

    // get format
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

    // same as in TextureN::loadFromFile
    unsigned int texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), imageWidth, imageHeight, 0, internalFormat,
                 GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLfloat maxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
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

    // free texture data
    stbi_image_free(data);

    // return texture id
    return texID;
}

void Model::setDefaultBoneData(MeshN::Vertex& vertex)
{
    for (unsigned int i{0}; i < MAX_BONE_INFLUENCE; ++i)
    {
        vertex.boneIDs[i] = -1;
        vertex.weights[i] = 0.0f;
    }
}

void Model::setVertexBoneData(MeshN::Vertex& vertex, const int boneID, const float weight)
{
    for (unsigned int i{0}; i < MAX_BONE_INFLUENCE; ++i)
    {
        if (vertex.boneIDs[i] < 0)
        {
            vertex.boneIDs[i] = boneID;
            vertex.weights[i] = weight;
            break;
        }
    }
}

// check if vertex weights are zeroed
bool Model::checkVertexWeights(const MeshN::Vertex& vertex)
{
    float totalWeight{0.0f};
    for (std::size_t w{0}; w < MAX_BONE_INFLUENCE; ++w)
    {
        totalWeight += vertex.weights[w];
    }
    return totalWeight > 0.001f;
}

void Model::extractBoneWeights(std::vector<MeshN::Vertex>& vertices, const aiMesh* mesh)
{
    // get weights for each bone
    for (unsigned int boneIdx{0}; boneIdx < mesh->mNumBones; ++boneIdx)
    {
        int boneID{-1};
        const std::string boneName{mesh->mBones[boneIdx]->mName.C_Str()};
        if (m_boneInfoMap.find(boneName) == m_boneInfoMap.end())
        {
            const MeshN::BoneInfo boneInfo{m_boneCounter, Util::convertMatrixGLM(mesh->mBones[boneIdx]->mOffsetMatrix)};
            m_boneInfoMap[boneName] = boneInfo;
            boneID = m_boneCounter;
            ++m_boneCounter;
        }
        else
        {
            boneID = m_boneInfoMap[boneName].id;
        }

        assert(boneID != -1);
        aiVertexWeight* weights{mesh->mBones[boneIdx]->mWeights};
        for (unsigned int weightIdx{0}; weightIdx < mesh->mBones[boneIdx]->mNumWeights; ++weightIdx)
        {
            const unsigned int vertexID{weights[weightIdx].mVertexId};
            const float weight{weights[weightIdx].mWeight};
            assert(vertexID < vertices.size());
            setVertexBoneData(vertices[vertexID], boneID, weight);
        }
    }

    // fix vertex spikes (try and get rid of any vertices with zeroed weights)
    for (std::size_t i{0}; i < vertices.size(); ++i)
    {
        if (!checkVertexWeights(vertices[i]))
        {
            // try and match vertex to nearest vertex with a bone
            float minDistance2{std::numeric_limits<float>::max()};
            int minIdx{-1};
            for (std::size_t j{0}; j < vertices.size(); ++j)
            {
                if (j == i)
                    continue;

                if (!checkVertexWeights(vertices[j]))
                    continue;

                const float dx{vertices[i].position.x - vertices[j].position.x};
                const float dy{vertices[i].position.y - vertices[j].position.y};
                const float dz{vertices[i].position.z - vertices[j].position.z};
                const float dist2{dx * dx + dy * dy + dz * dz};
                if (dist2 < minDistance2)
                {
                    minDistance2 = dist2;
                    minIdx = static_cast<int>(j);
                }
            }

            if (minIdx >= 0)
            {
                // get bone info from nearest good vertex
                for (std::size_t v{0}; v < MAX_BONE_INFLUENCE; ++v)
                {
                    vertices[i].boneIDs[v] = vertices[minIdx].boneIDs[v];
                    vertices[i].weights[v] = vertices[minIdx].weights[v];
                }
            }
            else if (m_boneCounter > 0)
            {
                // fallback
                vertices[i].boneIDs[0] = 0;
                vertices[i].weights[0] = 1.0f;
                for (std::size_t v{1}; v < MAX_BONE_INFLUENCE; ++v)
                {
                    vertices[i].boneIDs[v] = -1;
                    vertices[i].weights[v] = 0.0f;
                }
            }
        }
    }
}

void Model::handleTransparentTextures(const aiScene* scene)
{
    for (std::size_t i{0}; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh{scene->mMeshes[i]};
        const aiMaterial* material{scene->mMaterials[mesh->mMaterialIndex]};
        aiString alphaMode; // get the GLTF_ALPHAMODE

        if (material->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == AI_SUCCESS)
        {
            const std::string mode{alphaMode.C_Str()};
            if (mode == "BLEND")
            {
                m_meshes[i].setBlendMode(MeshN::BLEND_TRANSPARENT);
            }
            else if (mode == "MASK")
            {
                m_meshes[i].setBlendMode(MeshN::BLEND_MASK);
            }
            else
            {
                m_meshes[i].setBlendMode(MeshN::BLEND_OPAQUE);
            }
        }
    }
}

void Model::loadEmissiveFactor(const aiMaterial* mat, glm::vec3& emissiveFactor, float& emissiveIntensity)
{
    aiColor3D emissiveFactorColor{0.0f, 0.0f, 0.0f};
    if (mat->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveFactorColor) == AI_SUCCESS)
    {
        emissiveFactor.r = emissiveFactorColor.r;
        emissiveFactor.g = emissiveFactorColor.g;
        emissiveFactor.b = emissiveFactorColor.b;
    }

    if (mat->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissiveIntensity) != AI_SUCCESS)
        emissiveIntensity = 1.0f;
}

void Model::loadBaseColor(const aiMaterial* mat, glm::vec4& baseColor)
{
    aiColor4D baseColorFactor{1.0f, 1.0f, 1.0f, 1.0f};
    if (mat->Get(AI_MATKEY_BASE_COLOR, baseColorFactor) == AI_SUCCESS)
    {
        baseColor.r = baseColorFactor.r;
        baseColor.g = baseColorFactor.g;
        baseColor.b = baseColorFactor.b;
        baseColor.a = baseColorFactor.a;
    }
    else
    {
        baseColor = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    }
}

void Model::loadMetallicFactor(const aiMaterial* mat, float& metallicFactor)
{
    if (mat->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor) != AI_SUCCESS)
        metallicFactor = 1.0f;
}

void Model::loadRoughnessFactor(const aiMaterial* mat, float& roughnessFactor)
{
    if (mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor) != AI_SUCCESS)
        roughnessFactor = 1.0f;
}

// -------------- Model Manager -------------- //
ModelManager::ModelManager(EngineObject* parent) :
    EngineObject{"ModelManager", parent}
{
}

// load new model
void ModelManager::addModel(const std::string& name, const std::string& path, Arena* arena)
{
    // create new model and add it to arena
    Model* model{new Model{name, this}};
    arena->addObject(model);

    // add model
    if (!model->loadModel(path))
    {
        Util::beginError();
        std::cout << "MODEL_MANAGER::ADD_MODEL::ERROR: Failed to add model `" << name << "`";
        Util::endError();
    }
    else
    {
        m_models.insert(std::pair{name, model});
    }
}

Model* ModelManager::getModel(const std::string& name) const
{
    if (modelExists(name))
    {
        return m_models.find(name)->second;
    }
    Util::beginError();
    std::cout << "MODEL_MANAGER::GET_MODEL::ERROR: Model `" << name << "` does not exist!";
    Util::endError();
    return nullptr;
}

void ModelManager::renderModel(const Shader* shader, const std::string& name) const
{
    if (modelExists(name))
    {
        getModel(name)->render(shader);
    }
    else
    {
        std::cout << "MODEL_MANAGER::GET_MODEL::ERROR: Model `" << name << "` does not exist!\n";
    }
}

bool ModelManager::modelExists(const std::string& name) const { return m_models.find(name) != m_models.end(); }
