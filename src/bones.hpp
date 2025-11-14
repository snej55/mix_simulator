#ifndef BONES_H
#define BONES_H

#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>
#include <assimp/scene.h>
#include <map>
#include <string>
#include <vector>

#include "assimp/anim.h"
#include "assimp/postprocess.h"
#include "mesh.hpp"
#include "model.hpp"

namespace BonesN
{
    struct KeyPosition
    {
        glm::vec3 position;
        float timeStamp;
    };

    struct KeyRotation
    {
        glm::quat orientation;
        float timeStamp;
    };

    struct KeyScale
    {
        glm::vec3 scale;
        float timeStamp;
    };

    struct AssimpNodeData
    {
        glm::mat4 transform;
        std::string name;
        int childrenCount;
        std::vector<AssimpNodeData> children;
    };
}

class Bone
{
public:
    Bone(const std::string& name, int ID, const aiNodeAnim* channel);

    // read keyframes from aiNodeAnim
    void init(const aiNodeAnim* channel);

    // interpolates animation between keyframes based on time
    void update(float time);

    // get current index of position to interpolate on
    [[nodiscard]] int getPositionIndex(float time) const;

    // get current index of rotation to interpolate on
    [[nodiscard]] int getRotationIndex(float time) const;

    // get current index of scale to interpolate on
    [[nodiscard]] int getScaleIndex(float time) const;

    // getters
    [[nodiscard]] const glm::mat4& getLocalTransform() const {return m_localTransform;}
    [[nodiscard]] std::string_view getBoneName() const {return m_name;}
    [[nodiscard]] int getBoneID() const {return m_ID;}

private:
    std::string m_name;
    int m_ID;

    glm::mat4 m_localTransform{1.0f};

    std::vector<BonesN::KeyPosition> m_positions{};
    std::vector<BonesN::KeyRotation> m_rotations{};
    std::vector<BonesN::KeyScale> m_scales{};

    int m_numPositions{};
    int m_numRotations{};
    int m_numScales{};

    // get normalized value for lerp & slerp
    static float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

    [[nodiscard]] glm::mat4 interpolatePosition(float animationTime) const;

    [[nodiscard]] glm::mat4 interpolateRotation(float animationTime) const;

    [[nodiscard]] glm::mat4 interpolateScaling(float animationTime) const;
};


class BoneAnimation
{
public:
    BoneAnimation() = default;
    BoneAnimation(const std::string& animationPath, Model* model);

    ~BoneAnimation() = default;

    Bone* findBone(const std::string& name);

    [[nodiscard]] float getTicksPerSecond() const {return m_tps;}
    [[nodiscard]] float getDuration() const {return m_duration;}
    [[nodiscard]] const BonesN::AssimpNodeData& getRootNode() const {return m_rootNode;}
    [[nodiscard]] const std::map<std::string, MeshN::BoneInfo>& getBoneInfoMap() {return m_boneInfoMap;}

private:
    float m_duration{0.0f};
    int m_tps{0}; // ticks per second

    std::vector<Bone> m_bones{};
    BonesN::AssimpNodeData m_rootNode{};
    std::map<std::string, MeshN::BoneInfo> m_boneInfoMap{};

    void readMissingBones(const aiAnimation* animation, Model* model);
    void readHeirarchyData(BonesN::AssimpNodeData& dest, const aiNode* src);
};

// the actual animation class
class BoneAnimator
{
public:
    BoneAnimator(BoneAnimation* animation);

    void updateAnimation(float dt);
    void playAnimation(BoneAnimation* animation);
    void calculateBoneTransform(const BonesN::AssimpNodeData* node, const glm::mat4& parentTransform);

    [[nodiscard]] const std::vector<glm::mat4>& getFinalBoneMatrices() const {return m_finalBoneMatrices;}
    
private:
    BoneAnimation* m_currentAnimation;
    float m_currentTime;

    float m_deltaTime{1.0f};
    std::vector<glm::mat4> m_finalBoneMatrices{};
};

#endif
