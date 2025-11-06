//
// Created by Jens Kromdijk on 23/06/25.
//

#include <STB/stb_image.h>
#include <assimp/postprocess.h>
#include <glad/glad.h>
#include <mikktspace.h>

#include "assimp/material.h"
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

    // overkill log
    int numVertices{};
    for (const Mesh& mesh : m_meshes)
    {
        numVertices += static_cast<int>(mesh.getVertices().size());
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
    // extractBoneWeights(vertices, mesh, scene);

    // materials
    aiMaterial* material{scene->mMaterials[mesh->mMaterialIndex]};

    // use custom glTF Material Output node in blender for ambient occlusion texture
    std::vector<MeshN::Texture> aoMaps{
        loadMaterialTextures(scene, material, aiTextureType_LIGHTMAP, MeshN::TEXTURE_AO)};
    textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
    // albedo texture
    std::vector<MeshN::Texture> albedoMaps{
        loadMaterialTextures(scene, material, aiTextureType_BASE_COLOR, MeshN::TEXTURE_ALBEDO)};
    textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());
    // metallic texture (b-channel of metallic-roughness texture)
    std::vector<MeshN::Texture> metallicMaps{
        loadMaterialTextures(scene, material, aiTextureType_METALNESS, MeshN::TEXTURE_METALLIC)};
    textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());
    // roughness texture (g-channel)
    std::vector<MeshN::Texture> roughnessMaps{
        loadMaterialTextures(scene, material, aiTextureType_GLTF_METALLIC_ROUGHNESS, MeshN::TEXTURE_ROUGHNESS)};
    textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
    // normal map texture
    std::vector<MeshN::Texture> normalMaps{
        loadMaterialTextures(scene, material, aiTextureType_NORMALS, MeshN::TEXTURE_NORMAL)};
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    return Mesh{vertices, indices, textures};
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

void Model::setVertexBoneData(MeshN::Vertex& vertex, int boneID, float weight)
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

void Model::extractBoneWeights(std::vector<MeshN::Vertex>& vertices, const aiMesh* mesh, const aiScene* scene)
{
    for (unsigned int boneIdx{0}; boneIdx < mesh->mNumBones; ++boneIdx)
    {
        int boneID{-1};
        const std::string boneName {mesh->mBones[boneIdx]->mName.C_Str()};
        if (m_boneInfoMap.find(boneName) == m_boneInfoMap.end())
        {
            const MeshN::BoneInfo boneInfo {
                m_boneCounter,
                Util::convertMatrixGLM(mesh->mBones[boneIdx]->mOffsetMatrix)
            };
            m_boneInfoMap[boneName] = boneInfo;
            boneID = m_boneCounter;
            ++m_boneCounter;
        } else
        {
            boneID = m_boneInfoMap[boneName].id;
        }

        assert(boneID != -1);
        aiVertexWeight* weights {mesh->mBones[boneIdx]->mWeights};
        for (unsigned int weightIdx{0}; weightIdx < mesh->mBones[boneIdx]->mNumWeights; ++weightIdx)
        {
            const unsigned int vertexID{weights[weightIdx].mVertexId};
            const float weight{weights[weightIdx].mWeight};
            assert(vertexID <= vertices.size());
            setVertexBoneData(vertices[vertexID], boneID, weight);
        }
    }
}

// -------------- Model Manager -------------- //
ModelManager::ModelManager(EngineObject* parent) : EngineObject{"ModelManager", parent} {}

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
