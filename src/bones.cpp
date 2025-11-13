//
// Created by Jens Kromdijk on 07/11/2025.

#include "bones.hpp"
#include <cmath>
#include "assimp/anim.h"
#include "mesh.hpp"
#include "util.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <algorithm>

Bone::Bone(const std::string& name, const int ID, const aiNodeAnim* channel) : m_name{name}, m_ID{ID} { init(channel); }

void Bone::init(const aiNodeAnim* channel)
{
    // load positions
    m_numPositions = static_cast<int>(channel->mNumPositionKeys);
    for (std::size_t positionIdx{0}; positionIdx < m_numPositions; ++positionIdx)
    {
        aiVector3D aiPosition{channel->mPositionKeys[positionIdx].mValue};
        const float timeStamp{static_cast<float>(channel->mPositionKeys[positionIdx].mTime)};
        BonesN::KeyPosition data{
            Util::convertVectorGLM(aiPosition),
            timeStamp,
        };
        m_positions.emplace_back(data);
    }

    // load rotations
    m_numRotations = static_cast<int>(channel->mNumRotationKeys);
    for (std::size_t rotationIdx{0}; rotationIdx < m_numRotations; ++rotationIdx)
    {
        aiQuaternion aiRotation{channel->mRotationKeys[rotationIdx].mValue};
        const float timeStamp{static_cast<float>(channel->mRotationKeys[rotationIdx].mTime)};
        BonesN::KeyRotation data{
            Util::convertQuaternionGLM(aiRotation),
            timeStamp,
        };
        m_rotations.emplace_back(data);
    }

    // load scales
    m_numScales = static_cast<int>(channel->mNumScalingKeys);
    for (std::size_t scaleIdx{0}; scaleIdx < m_numScales; ++scaleIdx)
    {
        aiVector3D aiScale{channel->mScalingKeys[scaleIdx].mValue};
        const float timeStamp{static_cast<float>(channel->mScalingKeys[scaleIdx].mTime)};
        BonesN::KeyScale data{Util::convertVectorGLM(aiScale), timeStamp};
        m_scales.emplace_back(data);
    }
}

void Bone::update(const float time)
{
    // interpolate based on animationTime
    const glm::mat4 translation{interpolatePosition(time)};
    const glm::mat4 rotation{interpolateRotation(time)};
    const glm::mat4 scale{interpolateScaling(time)};
    m_localTransform = translation * rotation * scale;
}

int Bone::getPositionIndex(const float time) const
{
    for (std::size_t i{0}; i < m_numPositions; ++i)
    {
        if (time < m_positions[i].timeStamp)
        {
            return static_cast<int>(i);
        }
    }
    Util::beginError();
    std::cout << "BONE::GET_POSITION_INDEX::ERROR: Could not find animation time stamp!" << std::endl;
    Util::endError();
    return 0;
}

int Bone::getRotationIndex(const float time) const
{
    for (std::size_t i{0}; i < m_numRotations; ++i)
    {
        if (time < m_rotations[i].timeStamp)
        {
            return static_cast<int>(i);
        }
    }
    Util::beginError();
    std::cout << "BONE::GET_ROTATION_INDEX::ERROR: Could not find animation time stamp!" << std::endl;
    Util::endError();
    return 0;
}

int Bone::getScaleIndex(const float time) const
{
    for (std::size_t i{0}; i < m_numScales; ++i)
    {
        if (time < m_scales[i].timeStamp)
        {
            return static_cast<int>(i);
        }
    }
    Util::beginError();
    std::cout << "BONE::GET_SCALE_INDEX::ERROR: Could not find animation time stamp!" << std::endl;
    Util::endError();
    return 0;
}

float Bone::getScaleFactor(const float lastTimeStamp, const float nextTimeStamp, const float animationTime)
{
    float scaleFactor{0.0f};
    const float midwayLength{animationTime - lastTimeStamp};
    const float framesDiff{nextTimeStamp - lastTimeStamp};
    scaleFactor = midwayLength / framesDiff;
    return scaleFactor;
}

glm::mat4 Bone::interpolatePosition(const float animationTime) const
{
    if (m_numPositions == 1)
        return glm::translate(glm::mat4{1.0f}, m_positions[0].position);

    // get indices
    const int position0Idx{getPositionIndex(animationTime)};
    const int position1Idx{position0Idx + 1};
    // and the scale factor
    const float scaleFactor{
        getScaleFactor(m_positions[position0Idx].timeStamp, m_positions[position1Idx].timeStamp, animationTime)};
    // and use them to get the final position
    const glm::vec3 finalPosition{
        glm::mix(m_positions[position0Idx].position, m_positions[position1Idx].position, scaleFactor)};
    return glm::translate(glm::mat4{1.0f}, finalPosition);
}

glm::mat4 Bone::interpolateRotation(const float animationTime) const
{
    if (m_numRotations == 1)
        return glm::toMat4(glm::normalize(m_rotations[0].orientation));

    const int rotation0Idx{getRotationIndex(animationTime)};
    const int rotation1Idx{rotation0Idx + 1};
    const float scaleFactor{
        getScaleFactor(m_rotations[rotation0Idx].timeStamp, m_rotations[rotation1Idx].timeStamp, animationTime)};
    glm::quat finalRotation{
        glm::slerp(m_rotations[rotation0Idx].orientation, m_rotations[rotation1Idx].orientation, scaleFactor)};
    finalRotation = glm::normalize(finalRotation);
    return glm::toMat4(finalRotation);
}

glm::mat4 Bone::interpolateScaling(const float animationTime) const
{
    if (m_numScales == 1)
        return glm::scale(glm::mat4{1.0f}, m_scales[0].scale);

    const int scale0Idx{getScaleIndex(animationTime)};
    const int scale1Idx{scale0Idx + 1};
    const float scaleFactor{
        getScaleFactor(m_scales[scale0Idx].timeStamp, m_scales[scale1Idx].timeStamp, animationTime)};
    const glm::vec3 finalScale{glm::mix(m_scales[scale0Idx].scale, m_scales[scale1Idx].scale, scaleFactor)};
    return glm::scale(glm::mat4{1.0f}, finalScale);
}

BoneAnimation::BoneAnimation(const std::string& animationPath, Model* model)
{
    // load model
    Assimp::Importer importer;
    const aiScene* scene{importer.ReadFile(animationPath, aiProcess_Triangulate)};
    assert(scene && scene->mRootNode);

    const aiAnimation* anim{scene->mAnimations[0]};
    m_duration = anim->mDuration;
    m_tps = anim->mTicksPerSecond;
}

// find bone using std::find_if
Bone* BoneAnimation::findBone(const std::string& name)
{
    auto iter{
        std::find_if(m_bones.begin(), m_bones.end(), [&](const Bone& bone) { return bone.getBoneName() == name; })};
    if (iter == m_bones.end())
        return nullptr;
    else
        return &(*iter);
}

void BoneAnimation::readMissingBones(const aiAnimation* animation, Model* model)
{
    const unsigned int size {animation->mNumChannels};
    
    std::map<std::string, MeshN::BoneInfo>& boneInfoMap {model->getBoneInfoMap()};
    int& boneCount {model->getBoneCounter()};

    // read the channels
    for (std::size_t i{0}; i < size; ++i)
    {
	aiNodeAnim* channel {animation->mChannels[i]};
	std::string boneName {channel->mNodeName.data};

	if (boneInfoMap.find(boneName) == boneInfoMap.end())
	{
	    boneInfoMap[boneName].id = boneCount;
	    ++boneCount;
	}
	m_bones.push_back(Bone{channel->mNodeName.data, boneInfoMap[channel->mNodeName.data].id, channel});
    }

    m_boneInfoMap = boneInfoMap;
}

void BoneAnimation::readHeirarchyData(BonesN::AssimpNodeData& dest, const aiNode* src)
{
    assert(src);

    dest.name = src->mName.data;
    dest.transform = Util::convertMatrixGLM(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (std::size_t i{0}; i < src->mNumChildren; ++i)
    {
	BonesN::AssimpNodeData data;
	readHeirarchyData(data, src->mChildren[i]);
	dest.children.push_back(data);
    }
}

// the actual animator class
BoneAnimator::BoneAnimator(BoneAnimation* animation)
 : m_currentAnimation{animation}, m_currentTime{0.0f}
{
    m_finalBoneMatrices.reserve(100);
    for (std::size_t i{0}; i < 100; ++i)
    {
	m_finalBoneMatrices.push_back(glm::mat4{1.0f});
    }
}

void BoneAnimator::updateAnimation(const float dt)
{
    m_deltaTime = dt;
    if (m_currentAnimation)
    {
	m_currentTime += m_currentAnimation->getTicksPerSecond() * dt;
	m_currentTime = std::fmod(m_currentTime, m_currentAnimation->getDuration());
	calculateBoneTransform(&m_currentAnimation->getRootNode(), glm::mat4{1.0f});
    }
}

void BoneAnimator::playAnimation(BoneAnimation* animation)
{
    m_currentAnimation = animation;
    m_currentTime = 0.0f;
}

void BoneAnimator::calculateBoneTransform(const BonesN::AssimpNodeData* node, glm::mat4 parentTransform)
{
    glm::mat4 nodeTransform{node->transform};
    Bone* bone {m_currentAnimation->findBone(node->name)};
    if (bone)
    {
	bone->update(m_currentTime);
	nodeTransform = bone->getLocalTransform();
    }

    glm::mat4 globalTranformation {parentTransform * nodeTransform};

    std::map<std::string, MeshN::BoneInfo> boneInfoMap {m_currentAnimation->getBoneInfoMap()};
    if (boneInfoMap.find(node->name) != boneInfoMap.end())
    {
	m_finalBoneMatrices[boneInfoMap[node->name].id] = globalTranformation * boneInfoMap[node->name].offset;
    }

    for (std::size_t i{0}; i < node->childrenCount; ++i)
	calculateBoneTransform(&node->children[i], globalTranformation);
}
