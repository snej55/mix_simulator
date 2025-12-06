//
// Created by Jens Kromdijk on 06/12/2025.
//

#include "bounds.hpp"

void Bounds::Transform::computeModelMatrix()
{
    m_modelMat = getLocalModelMat();
    m_dirty = false;
}

void Bounds::Transform::computeModelMatrix(const glm::mat4& parentGlobalModelMatrix)
{
    m_modelMat = parentGlobalModelMatrix * getLocalModelMat();
    m_dirty = false;
}

void Bounds::Transform::setLocalPosition(const glm::vec3& localPosition)
{
    m_pos = localPosition;
    m_dirty = true;
}

glm::mat4 Bounds::Transform::getLocalModelMat() const
{
    const glm::mat4 transformX {glm::rotate(glm::mat4{1.0f}, glm::radians(m_eulerRot.x), glm::vec3{1.0f, 0.0f, 0.0f})};
    const glm::mat4 transformY {glm::rotate(glm::mat4{1.0f}, glm::radians(m_eulerRot.y), glm::vec3{0.0f, 1.0f, 0.0f})};
    const glm::mat4 transformZ {glm::rotate(glm::mat4{1.0f}, glm::radians(m_eulerRot.z), glm::vec3{0.0f, 0.0f, 1.0f})};

    const glm::mat4 rotationMatrix = transformY * transformX * transformZ;
    return glm::translate(glm::mat4{1.0f}, m_pos) * rotationMatrix * glm::scale(glm::mat4{1.0f}, m_scale);
}

Bounds::Plane::Plane(const glm::vec3& p1, const glm::vec3& norm)
    : normal{glm::normalize(norm)}, distance{glm::dot(normal, p1)}
{
}

float Bounds::Plane::getSignedDistance(const glm::vec3& p) const
{
    return glm::dot(normal, p) - distance;
}

Bounds::Frustum Bounds::createFrustum(const Camera& cam, float aspectR, float fovY, float zNear, float zFar)
{

    const float halfVSide {zFar * std::tanf(fovY * 0.5f)};
    const float halfHSide {halfVSide * aspectR};
    const glm::vec3 frontMultFar {zFar * cam.getFront()};

    Frustum frustum;
    frustum.nearFace = {cam.getPosition() + zNear * cam.getFront(), cam.getFront()};
    frustum.farFace = {cam.getPosition() + frontMultFar, -cam.getFront()};
    frustum.rightFace = {cam.getPosition(), glm::cross(frontMultFar - cam.getRight() * halfHSide, cam.getUp())};
    frustum.leftFace = {cam.getPosition(), glm::cross(cam.getUp(), frontMultFar + cam.getRight() * halfHSide)};
    frustum.topFace = {cam.getPosition(), glm::cross(cam.getRight(), frontMultFar - cam.getUp() * halfVSide)};
    frustum.bottomFace = {cam.getPosition(), glm::cross(frontMultFar + cam.getUp() * halfVSide, cam.getRight())};

    return frustum;
}
