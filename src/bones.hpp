#ifndef BONES_H
#define BONES_H

#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>
#include <assimp/scene.h>
#include <string>
#include <vector>

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
    int getPositionIndex(int index);

    // get current index of rotation to interpolate on
    int getRotationIndex(int index);

    // get current index of scale to interpolate on
    int getScaleIndex(int index);

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
    float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

    glm::mat4 interpolatePosition(float animationTime);

    glm::mat4 interpolateRotation(float animationTime);

    glm::mat4 interpolateScaling(float animationTime);
};

#endif
