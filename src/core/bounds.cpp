//
// Created by Jens Kromdijk on 06/12/2025.
//

#include <limits>

#include "bounds.hpp"
#include "mesh.hpp"

Bounds::Transform::Transform(const glm::vec3& pos, const glm::vec3& rotation, const glm::vec3& scale) :
    m_pos{pos}, m_eulerRot{rotation}, m_scale{scale}
{
}

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

void Bounds::Transform::setLocalRotation(const glm::vec3& localRotation)
{
    m_eulerRot = localRotation;
    m_dirty = true;
}

void Bounds::Transform::setLocalScale(const glm::vec3& localScale)
{
    m_scale = localScale;
    m_dirty = true;
}

void Bounds::Transform::setPivotOffset(const glm::vec3& pivotOffset)
{
    m_pivotOffset = pivotOffset;
    m_dirty = true;
}

glm::vec3 Bounds::Transform::getGlobalPivotOffset() const
{
    const glm::vec3 scaled{m_pivotOffset * m_scale};
    return getRight() * scaled.x + getUp() * scaled.y + getForward() * scaled.z;
}


glm::mat4 Bounds::Transform::getLocalModelMat() const
{
    const glm::mat4 transformX{glm::rotate(glm::mat4{1.0f}, glm::radians(m_eulerRot.x), glm::vec3{1.0f, 0.0f, 0.0f})};
    const glm::mat4 transformY{glm::rotate(glm::mat4{1.0f}, glm::radians(m_eulerRot.y), glm::vec3{0.0f, 1.0f, 0.0f})};
    const glm::mat4 transformZ{glm::rotate(glm::mat4{1.0f}, glm::radians(m_eulerRot.z), glm::vec3{0.0f, 0.0f, 1.0f})};

    const glm::mat4 rotationMatrix = transformZ * transformY * transformX;

    return glm::translate(glm::mat4{1.0f}, m_pos) * rotationMatrix * glm::scale(glm::mat4{1.0f}, m_scale) *
        glm::translate(glm::mat4{1.0f}, m_pivotOffset);
}

Bounds::Plane::Plane(const glm::vec3& aP1, const glm::vec3& norm) :
    normal{glm::normalize(norm)}, distance{glm::dot(normal, aP1)}, p1{aP1}
{
}

float Bounds::Plane::getSignedDistance(const glm::vec3& p) const { return glm::dot(normal, p) - distance; }

Bounds::Frustum Bounds::createFrustum(const Camera* cam, const float aspectR, const float fovY, const float zNear,
                                      const float zFar)
{

    const float halfVSide{zFar * std::tanf(fovY * 0.5f)};
    const float halfHSide{halfVSide * aspectR};
    const glm::vec3 frontMultFar{zFar * cam->getFront()};

    Frustum frustum;
    frustum.nearFace = {cam->getPosition() + zNear * cam->getFront(), cam->getFront()};
    frustum.farFace = {cam->getPosition() + frontMultFar, -cam->getFront()};
    frustum.rightFace = {cam->getPosition(), glm::cross(frontMultFar - cam->getRight() * halfHSide, cam->getUp())};
    frustum.leftFace = {cam->getPosition(), glm::cross(cam->getUp(), frontMultFar + cam->getRight() * halfHSide)};
    frustum.topFace = {cam->getPosition(), glm::cross(cam->getRight(), frontMultFar - cam->getUp() * halfVSide)};
    frustum.bottomFace = {cam->getPosition(), glm::cross(frontMultFar + cam->getUp() * halfVSide, cam->getRight())};

    return frustum;
}

Bounds::Sphere::Sphere(const glm::vec3& aCenter, const float aRadius) : Volume{}, center{aCenter}, radius{aRadius} {}

bool Bounds::Sphere::onForwardPlane(const Plane& plane) const { return plane.getSignedDistance(center) > -radius; }

bool Bounds::Sphere::onFrustum(const Frustum& camFrustum, const Transform& modelTransform) const
{
    const glm::vec3 globalScale{modelTransform.getGlobalScale()};
    const glm::vec3 globalCenter{modelTransform.getModelMat() * glm::vec4{center, 1.f}};
    const float maxScale{std::max(std::max(globalScale.x, globalScale.y), globalScale.z)};

    Sphere globalSphere{globalCenter, radius * (maxScale * 0.5f)};

    return (globalSphere.onForwardPlane(camFrustum.leftFace) && globalSphere.onForwardPlane(camFrustum.rightFace) &&
            globalSphere.onForwardPlane(camFrustum.farFace) && globalSphere.onForwardPlane(camFrustum.nearFace) &&
            globalSphere.onForwardPlane(camFrustum.topFace) && globalSphere.onForwardPlane(camFrustum.bottomFace));
}

bool Bounds::Sphere::collidePoint(const glm::vec3& point) const { return glm::length(point - center) <= radius; }

bool Bounds::Sphere::collideSphere(const Sphere& other) const
{
    return glm::length(other.center - center) <= radius + other.radius;
}

Bounds::Sphere Bounds::generateSphereBV(const Model* model)
{
    glm::vec3 minAABB{glm::vec3{std::numeric_limits<float>::max()}};
    glm::vec3 maxAABB{glm::vec3{std::numeric_limits<float>::lowest()}};

    for (const Mesh* mesh : model->getOpaqueMeshes())
    {
        for (const MeshN::Vertex& vertex : mesh->getVertices())
        {
            minAABB.x = std::min(minAABB.x, vertex.position.x);
            minAABB.y = std::min(minAABB.y, vertex.position.y);
            minAABB.z = std::min(minAABB.z, vertex.position.z);

            maxAABB.x = std::max(maxAABB.x, vertex.position.x);
            maxAABB.y = std::max(maxAABB.y, vertex.position.y);
            maxAABB.z = std::max(maxAABB.z, vertex.position.z);
        }
    }

    // repeat for transparent meshes
    for (const Mesh* mesh : model->getTransparentMeshes())
    {
        for (const MeshN::Vertex& vertex : mesh->getVertices())
        {
            minAABB.x = std::min(minAABB.x, vertex.position.x);
            minAABB.y = std::min(minAABB.y, vertex.position.y);
            minAABB.z = std::min(minAABB.z, vertex.position.z);

            maxAABB.x = std::max(maxAABB.x, vertex.position.x);
            maxAABB.y = std::max(maxAABB.y, vertex.position.y);
            maxAABB.z = std::max(maxAABB.z, vertex.position.z);
        }
    }

    return Sphere{(maxAABB + minAABB) * 0.5f, glm::length(minAABB - maxAABB)};
}

Bounds::AABB::AABB(const glm::vec3& min, const glm::vec3& max) :
    Volume{}, center{(min + max) * 0.5f},
    extents{std::abs(max.x - min.x) * 0.5f, std::abs(max.y - min.y) * 0.5f, std::abs(max.z - min.z) * 0.5f}
{
}

Bounds::AABB::AABB(const glm::vec3& aCenter, float eX, float eY, float eZ) :
    Volume{}, center{aCenter}, extents{eX, eY, eZ}
{
}

bool Bounds::AABB::onForwardPlane(const Plane& plane) const
{
    const float r{extents.x * std::abs(plane.normal.x) + extents.y * std::abs(plane.normal.y) +
                  extents.z * std::abs(plane.normal.z)};
    return -r <= plane.getSignedDistance(center);
}

bool Bounds::AABB::onFrustum(const Frustum& camFrustum, const Transform& transform) const
{
    const glm::vec3 globalCenter{transform.getModelMat() * glm::vec4{center, 1.f}};

    // scaled orientation
    const glm::vec3 right{transform.getRight() * extents.x};
    const glm::vec3 up{transform.getUp() * extents.y};
    const glm::vec3 forward{transform.getForward() * extents.z};

    // new x extent
    const float nEX{std::abs(glm::dot(glm::vec3{1.0f, 0.0f, 0.0f}, right)) +
                    std::abs(glm::dot(glm::vec3{1.0f, 0.0f, 0.0f}, up)) +
                    std::abs(glm::dot(glm::vec3{1.0f, 0.0f, 0.0f}, forward))};
    // new y extent
    const float nEY{std::abs(glm::dot(glm::vec3{0.0f, 1.0f, 0.0f}, right)) +
                    std::abs(glm::dot(glm::vec3{0.0f, 1.0f, 0.0f}, up)) +
                    std::abs(glm::dot(glm::vec3{0.0f, 1.0f, 0.0f}, forward))};
    // new z extent
    const float nEZ{std::abs(glm::dot(glm::vec3{0.0f, 0.0f, 1.0f}, right)) +
                    std::abs(glm::dot(glm::vec3{0.0f, 0.0f, 1.0f}, up)) +
                    std::abs(glm::dot(glm::vec3{0.0f, 0.0f, 1.0f}, forward))};

    const AABB globalAABB{globalCenter, nEX, nEY, nEZ};
    return (globalAABB.onForwardPlane(camFrustum.leftFace) && globalAABB.onForwardPlane(camFrustum.rightFace) &&
            globalAABB.onForwardPlane(camFrustum.farFace) && globalAABB.onForwardPlane(camFrustum.nearFace) &&
            globalAABB.onForwardPlane(camFrustum.topFace) && globalAABB.onForwardPlane(camFrustum.bottomFace));
}

bool Bounds::AABB::onFrustum(const Frustum& camFrustum, const Transform& transform, const float padding) const
{
    const glm::vec3 globalCenter{transform.getModelMat() * glm::vec4{center, 1.f}};

    // scaled orientation
    const glm::vec3 right{transform.getRight() * extents.x};
    const glm::vec3 up{transform.getUp() * extents.y};
    const glm::vec3 forward{transform.getForward() * extents.z};

    // new x extent
    const float nEX{std::abs(glm::dot(glm::vec3{1.0f, 0.0f, 0.0f}, right)) +
                    std::abs(glm::dot(glm::vec3{1.0f, 0.0f, 0.0f}, up)) +
                    std::abs(glm::dot(glm::vec3{1.0f, 0.0f, 0.0f}, forward))};
    // new y extent
    const float nEY{std::abs(glm::dot(glm::vec3{0.0f, 1.0f, 0.0f}, right)) +
                    std::abs(glm::dot(glm::vec3{0.0f, 1.0f, 0.0f}, up)) +
                    std::abs(glm::dot(glm::vec3{0.0f, 1.0f, 0.0f}, forward))};
    // new z extent
    const float nEZ{std::abs(glm::dot(glm::vec3{0.0f, 0.0f, 1.0f}, right)) +
                    std::abs(glm::dot(glm::vec3{0.0f, 0.0f, 1.0f}, up)) +
                    std::abs(glm::dot(glm::vec3{0.0f, 0.0f, 1.0f}, forward))};

    const AABB globalAABB{globalCenter, nEX + padding, nEY + padding, nEZ + padding};
    return (globalAABB.onForwardPlane(camFrustum.leftFace) && globalAABB.onForwardPlane(camFrustum.rightFace) &&
            globalAABB.onForwardPlane(camFrustum.farFace) && globalAABB.onForwardPlane(camFrustum.nearFace) &&
            globalAABB.onForwardPlane(camFrustum.topFace) && globalAABB.onForwardPlane(camFrustum.bottomFace));
}

bool Bounds::AABB::collidePoint(const glm::vec3& point) const
{
    const glm::vec3 min{center.x - extents.x, center.y - extents.y, center.z - extents.z};
    const glm::vec3 max{center.x + extents.x, center.y + extents.y, center.z + extents.z};
    return (min.x <= point.x && point.x <= max.x && min.y <= point.y && point.y <= max.y && min.z <= point.z &&
            point.z <= max.z);
}

bool Bounds::AABB::collideAABB(const AABB& other) const
{
    const glm::vec3 minA{center.x - extents.x, center.y - extents.y, center.z - extents.z};
    const glm::vec3 maxA{center.x + extents.x, center.y + extents.y, center.z + extents.z};

    const glm::vec3 minB{other.center.x - other.extents.x, other.center.y - other.extents.y,
                         other.center.z - other.extents.z};
    const glm::vec3 maxB{other.center.x + other.extents.x, other.center.y + other.extents.y,
                         other.center.z + other.extents.z};

    return (minA.x <= maxB.x && maxA.x >= minB.x && minA.y <= maxB.y && maxA.y >= minB.y && minA.z <= maxB.z &&
            maxA.z >= minB.z);
}

Bounds::AABB Bounds::generateAABB_BV(const Model* model, const glm::vec3& scale)
{
    glm::vec3 minAABB{glm::vec3{std::numeric_limits<float>::max()}};
    glm::vec3 maxAABB{glm::vec3{std::numeric_limits<float>::lowest()}};

    for (const Mesh* mesh : model->getOpaqueMeshes())
    {
        for (const MeshN::Vertex& vertex : mesh->getVertices())
        {
            minAABB.x = std::min(minAABB.x, vertex.position.x * scale.x);
            minAABB.y = std::min(minAABB.y, vertex.position.y * scale.y);
            minAABB.z = std::min(minAABB.z, vertex.position.z * scale.z);

            maxAABB.x = std::max(maxAABB.x, vertex.position.x * scale.x);
            maxAABB.y = std::max(maxAABB.y, vertex.position.y * scale.y);
            maxAABB.z = std::max(maxAABB.z, vertex.position.z * scale.z);
        }
    }

    // repeat for transparent meshes
    for (const Mesh* mesh : model->getTransparentMeshes())
    {
        for (const MeshN::Vertex& vertex : mesh->getVertices())
        {
            minAABB.x = std::min(minAABB.x, vertex.position.x * scale.x);
            minAABB.y = std::min(minAABB.y, vertex.position.y * scale.y);
            minAABB.z = std::min(minAABB.z, vertex.position.z * scale.z);

            maxAABB.x = std::max(maxAABB.x, vertex.position.x * scale.x);
            maxAABB.y = std::max(maxAABB.y, vertex.position.y * scale.y);
            maxAABB.z = std::max(maxAABB.z, vertex.position.z * scale.z);
        }
    }

    return AABB{minAABB, maxAABB};
}

Bounds::AABB Bounds::getFrustumBV(const Frustum& frustum, const Camera* cam, float zFar, float fovY, float aspectR)
{
    // get Frustum coordinates
    zFar = std::min(zFar, 1000.f);
    const float halfVSide{zFar * std::tanf(fovY * 0.5f)};
    const float halfHSide{halfVSide * aspectR};

    const glm::vec3 pos{cam->getPosition()};
    const glm::vec3 front{cam->getFront()};
    const glm::vec3 up{cam->getUp()};
    const glm::vec3 right{cam->getRight()};

    const glm::vec3 farCenter{pos + zFar * front};

    // far plane vectors
    const glm::vec3 farUp{halfVSide * up};
    const glm::vec3 farRight{halfHSide * right};

    // far plane corners
    const glm::vec3 farTopLeft{farCenter + farUp - farRight};
    const glm::vec3 farTopRight{farCenter + farUp + farRight};
    const glm::vec3 farBottomLeft{farCenter - farUp - farRight};
    const glm::vec3 farBottomRight{farCenter - farUp + farRight};

    // corners of near plane
    const float zNear{glm::distance(pos, frustum.nearFace.p1)};
    const float nearHalfVSide{zNear * std::tanf(fovY * 0.5f)};
    const float nearHalfHSide{nearHalfVSide * aspectR};

    const glm::vec3 nearCenter{pos + zNear * front};
    const glm::vec3 nearUp{nearHalfVSide * up};
    const glm::vec3 nearRight{nearHalfHSide * right};

    // actual corners
    const glm::vec3 nearTopLeft{nearCenter + nearUp - nearRight};
    const glm::vec3 nearTopRight{nearCenter + nearUp + nearRight};
    const glm::vec3 nearBottomLeft{nearCenter - nearUp - nearRight};
    const glm::vec3 nearBottomRight{nearCenter - nearUp + nearRight};

    const glm::vec3 corners[8]{farTopLeft,  farTopRight,  farBottomLeft,  farBottomRight,
                               nearTopLeft, nearTopRight, nearBottomLeft, nearBottomRight};

    // get actual bounding volume
    glm::vec3 minAABB{std::numeric_limits<float>::max()};
    glm::vec3 maxAABB{std::numeric_limits<float>::lowest()};
    for (std::size_t i{0}; i < 8; ++i)
    {
        const glm::vec3* corner{&corners[i]};
        minAABB.x = std::min(minAABB.x, corner->x);
        minAABB.y = std::min(minAABB.y, corner->y);
        minAABB.z = std::min(minAABB.z, corner->z);

        maxAABB.x = std::max(maxAABB.x, corner->x);
        maxAABB.y = std::max(maxAABB.y, corner->y);
        maxAABB.z = std::max(maxAABB.z, corner->z);
    }

    return {minAABB, maxAABB};
}

Bounds::Rect2D::Rect2D(const glm::vec2& center, const glm::vec2& extents) : m_center{center}, m_extents{extents} {}

bool Bounds::Rect2D::collideRect(const Rect2D& other) const
{
    return (m_center.x - m_extents.x <= other.m_center.x + other.m_extents.x &&
            m_center.x + m_extents.x >= other.m_center.x - other.m_extents.x &&
            m_center.y - m_extents.y <= other.m_center.y + other.m_extents.y &&
            m_center.y + m_extents.y >= other.m_center.y - other.m_extents.y);
}

bool Bounds::Rect2D::collideAxis(const Rect2D& other, const bool x) const
{
    if (x)
    {
        return (m_center.x - m_extents.x <= other.m_center.x + other.m_extents.x &&
                m_center.x + m_extents.x >= other.m_center.x - other.m_extents.x);
    }
    return (m_center.y - m_extents.y <= other.m_center.y + other.m_extents.y &&
            m_center.y + m_extents.y >= other.m_center.y - other.m_extents.y);
}

float Bounds::Rect2D::calcOverlap(const Rect2D& other) const
{
    return std::max(0.f,
                    std::min(other.m_center.x + other.m_extents.x, m_center.x + m_extents.x) -
                        std::max(other.m_center.x - other.m_extents.x, m_center.x - m_extents.x)) *
        std::max(0.f,
                 std::min(other.m_center.y + other.m_extents.y, m_center.y + m_extents.y) -
                     std::max(other.m_center.y - other.m_extents.y, m_center.y - m_extents.y));
}
