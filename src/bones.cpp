//
// Created by Jens Kromdijk on 07/11/2025.

#include "bones.hpp"
#include "util.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel) :
    m_name{name}, m_ID{ID}
{
    init(channel);
}

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
        BonesN::KeyScale data{
            Util::convertVectorGLM(aiScale),
            timeStamp
        };
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
        getScaleFactor(m_positions[position0Idx].timeStamp, m_positions[position1Idx].timeStamp, animationTime)
    };
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
    const float scaleFactor
        {getScaleFactor(m_scales[scale0Idx].timeStamp, m_scales[scale1Idx].timeStamp, animationTime)};
    const glm::vec3 finalScale{glm::mix(m_scales[scale0Idx].scale, m_scales[scale1Idx].scale, scaleFactor)};
    return glm::scale(glm::mat4{1.0f}, finalScale);
}
