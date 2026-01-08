// Created by Jens Kromdijk 05-01-2026
// Setup JoltPhysics interface

#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Core/IssueReporting.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Core/Core.h>
#include <Jolt/Math/Real.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/Shape/SubShapeIDPair.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/MotionType.h>

#include <glm/gtc/quaternion.hpp>
#define GLM_FORCE_QUAT_DATA_WXYZ

#include <iostream>
#include <cstdarg>
#include <cassert>
#include <string_view>

#include "engine_types.hpp"
#include "bounds.hpp"
#include "util.hpp"

#define PHYSICS_TIME_STEP 0.0166667
#define PHYSICS_CONVEX_RADIUS 0.05f
#define PHYSICS_DEBUG_LOG

enum class BodyType
{
    STATIC = 0x0,
    DYNAMIC = 0x1,
    KINEMATIC = 0x2,
};

inline void getBodyType(const std::string_view bodyTypeStr, BodyType* bodyType)
{
    if (bodyTypeStr == "static")
    {
        *bodyType = BodyType::STATIC;
    }
    else if (bodyTypeStr == "dynamic")
    {
        *bodyType = BodyType::DYNAMIC;
    }
    else if (bodyTypeStr == "kinematic")
    {
        *bodyType = BodyType::KINEMATIC;
    }
    else
    {
        *bodyType = BodyType::STATIC;
    }
}

JPH_SUPPRESS_WARNINGS

// Jolt trace callback
static void TraceImpl(const char* inFMT, ...)
{
    // Format the message
    va_list list;
    va_start(list, inFMT);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);

    // Print to the TTY
    std::cout << buffer << std::endl;
}


#ifdef JPH_ENABLE_ASSERTS

// Jolt assert callback
static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine)
{
    std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "")
              << std::endl;

    return true;
}

#endif

namespace ObjectLayers
{
    static constexpr JPH::ObjectLayer NON_MOVING{0};
    static constexpr JPH::ObjectLayer MOVING{1};
    static constexpr JPH::ObjectLayer NUM_LAYERS{2};
}; // namespace ObjectLayers

class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
    {
        switch (inObject1)
        {
        case ObjectLayers::NON_MOVING:
            return inObject2 == ObjectLayers::MOVING;
        case ObjectLayers::MOVING:
            return true;
        default:
            JPH_ASSERT(false);
            return false;
        }
    }
};

namespace BroadPhaseLayers
{
    static constexpr JPH::BroadPhaseLayer NON_MOVING{0};
    static constexpr JPH::BroadPhaseLayer MOVING{1};
    static constexpr JPH::uint NUM_LAYERS{2};
} // namespace BroadPhaseLayers

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
    BPLayerInterfaceImpl()
    {
        // Create a mapping table from object to broad phase layer
        mObjectToBroadPhase[ObjectLayers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        mObjectToBroadPhase[ObjectLayers::MOVING] = BroadPhaseLayers::MOVING;
    }

    virtual uint GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }

    virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
    {
        JPH_ASSERT(inLayer < ObjectLayers::NUM_LAYERS);
        return mObjectToBroadPhase[inLayer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
    {
        switch ((JPH::BroadPhaseLayer::Type)inLayer)
        {
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
            return "NON_MOVING";
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
            return "MOVING";
        default:
            JPH_ASSERT(false);
            return "INVALID";
        }
    }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
    JPH::BroadPhaseLayer mObjectToBroadPhase[ObjectLayers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
    {
        switch (inLayer1)
        {
        case ObjectLayers::NON_MOVING:
            return inLayer2 == BroadPhaseLayers::MOVING;
        case ObjectLayers::MOVING:
            return true;
        default:
            JPH_ASSERT(false);
            return false;
        }
    }
};

#ifdef PHYSICS_DEBUG_LOG

class DebugContactListener : public JPH::ContactListener
{
    virtual JPH::ValidateResult OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2,
                                                  JPH::RVec3Arg inBaseOffset,
                                                  const JPH::CollideShapeResult& inCollisionResult) override
    {
        std::cout << "[Jolt] Contact vaolidate callback" << std::endl;
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2,
                                const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
    {
        std::cout << "[Jolt] A contact was added!" << std::endl;
    }

    virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2,
                                    const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
    {
        std::cout << "[Jolt] A contact was persisted!" << std::endl;
    }

    virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
    {
        std::cout << "[Jolt] A contact was removed!" << std::endl;
    }
};

class DebugBodyActivationListener : public JPH::BodyActivationListener
{
public:
    virtual void OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
    {
        std::cout << "[Jolt] A body has been activated!" << std::endl;
    }

    virtual void OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
    {
        std::cout << "[Jolt] A body went to sleep" << std::endl;
    }
};

#endif

// only simple boxes for now
class PhysicsBody final
{
public:
    PhysicsBody() = default;
    PhysicsBody(JPH::BodyInterface* bodyInterface, const Bounds::AABB& boundingBox, const glm::vec3& rotation,
                const BodyType bodyType) : m_bodyType{bodyType}
    {
        const JPH::Vec3 extents{Util::convertVectorJolt(boundingBox.extents)};
        JPH::BoxShapeSettings shapeSettings{extents, PHYSICS_CONVEX_RADIUS};
        JPH::Shape::ShapeResult result{shapeSettings.Create()};

        if (result.HasError())
        {
            Util::beginError();
            std::cout << "PHYSICS_BODY::ERROR: Failed to create box from " << boundingBox
                      << " Bodytype: " << static_cast<int>(bodyType);
            Util::endError();
            return;
        }

        JPH::Vec3 center{Util::convertVectorJolt(boundingBox.center)};

        JPH::EMotionType motionType;
        JPH::ObjectLayer layer;

        switch (bodyType)
        {
        case BodyType::STATIC:
            motionType = JPH::EMotionType::Static;
            layer = ObjectLayers::NON_MOVING;
            break;
        case BodyType::DYNAMIC:
            motionType = JPH::EMotionType::Dynamic;
            layer = ObjectLayers::MOVING;
            break;
        case BodyType::KINEMATIC:
            motionType = JPH::EMotionType::Kinematic;
            layer = ObjectLayers::MOVING;
            break;
        }

        JPH::Quat joltRotation{JPH::Quat::sEulerAngles({rotation.x, rotation.y, rotation.z})};
        JPH::BodyCreationSettings settings{result.Get(), center, joltRotation, motionType, layer};

        JPH::EActivation activation{(motionType == JPH::EMotionType::Static) ? JPH::EActivation::DontActivate
                                                                             : JPH::EActivation::Activate};

        m_bodyID = bodyInterface->CreateAndAddBody(settings, activation);
    }

    ~PhysicsBody() {}

    void syncTransform(Bounds::Transform& transform, const JPH::BodyInterface* bodyInterface) const
    {
        assert(!m_bodyID.IsInvalid());
        if (!bodyInterface->IsActive(m_bodyID))
            return;

        JPH::RVec3 jPos;
        JPH::Quat jRot;
        bodyInterface->GetPositionAndRotation(m_bodyID, jPos, jRot);

        transform.setLocalPosition(
            {static_cast<float>(jPos.GetX()), static_cast<float>(jPos.GetY()), static_cast<float>(jPos.GetZ())});

        glm::quat glmQuat{jRot.GetW(), jRot.GetX(), jRot.GetY(), jRot.GetZ()};
        glm::vec3 angles{glm::eulerAngles(glmQuat)};
        transform.setLocalRotation(glm::degrees(angles));
    }

    [[nodiscard]] const JPH::BodyID& getBodyID() const { return m_bodyID; }
    [[nodiscard]] BodyType getBodyType() const { return m_bodyType; }

private:
    BodyType m_bodyType;
    JPH::BodyID m_bodyID;
};

class JoltInstance final : public EngineObject
{
public:
    explicit JoltInstance(EngineObject* parent) : EngineObject{"JoltInstance", parent} {}

    ~JoltInstance()
    {
        JPH::UnregisterTypes();

        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }

    void init()
    {
        assert(!m_init);
        JPH::RegisterDefaultAllocator();

        JPH::Trace = TraceImpl;
        JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl);

        JPH::Factory::sInstance = new JPH::Factory();

        JPH::RegisterTypes();

        static JPH::TempAllocatorImpl tempAllocator{10 * 1024 * 1024};
        m_TempAllocator = &tempAllocator;

        static JPH::JobSystemThreadPool jobSystem{JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                                                  static_cast<int>(std::thread::hardware_concurrency() - 1)};
        m_JobSystem = &jobSystem;

        constexpr JPH::uint cMaxBodies{65536};
        constexpr JPH::uint cNumBodyMutexes{0};
        constexpr JPH::uint cMaxBodyPairs{65536};
        constexpr JPH::uint cMaxContactContraints{10240};

        static JPH::PhysicsSystem physicsSystem;
        m_PhysicsSystem = &physicsSystem;
        physicsSystem.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactContraints,
                           m_BroadPhaseLayerInterface, m_ObjectVsBroadphaseLayerFilter, m_ObjectVsObjectLayerFilter);

#ifdef PHYSICS_DEBUG_LOG
        physicsSystem.SetContactListener(&m_ContactListener);
        physicsSystem.SetBodyActivationListener(&m_BodyActivationListener);
#endif

        static JPH::BodyInterface& bodyInterface{physicsSystem.GetBodyInterface()};
        m_BodyInterface = &bodyInterface; // don't worry this works trust me

        m_init = true;
    }

    // deltatime is seconds
    void update(float deltaTime)
    {
        static float accumulator{0.0f};

        accumulator += deltaTime;
        accumulator = std::min(accumulator, 0.25f);

        while (accumulator >= PHYSICS_TIME_STEP)
        {
            m_PhysicsSystem->Update(PHYSICS_TIME_STEP, 1, m_TempAllocator, m_JobSystem);
            accumulator -= PHYSICS_TIME_STEP;
        }
    }

    [[nodiscard]] JPH::TempAllocatorImpl* getTempAllocator() const { return m_TempAllocator; }
    [[nodiscard]] JPH::JobSystemThreadPool* getJobSystem() const { return m_JobSystem; }
    [[nodiscard]] BPLayerInterfaceImpl& getBroadPhaseLayerInstance() { return m_BroadPhaseLayerInterface; }
    [[nodiscard]] ObjectVsBroadPhaseLayerFilterImpl& getObjectVsBroadphaseLayerFilter()
    {
        return m_ObjectVsBroadphaseLayerFilter;
    }
    [[nodiscard]] ObjectLayerPairFilterImpl& getObjectVsObjectLayerFilter() { return m_ObjectVsObjectLayerFilter; }

    [[nodiscard]] JPH::PhysicsSystem* getPhysicsSystem() const { return m_PhysicsSystem; }
    [[nodiscard]] JPH::BodyInterface* getBodyInterface() const { return m_BodyInterface; }

#ifdef PHYSICS_DEBUG_LOG
    [[nodiscard]] DebugContactListener& getContactListener() { return m_ContactListener; }
    [[nodiscard]] DebugBodyActivationListener& getBodyActivationListener() { return m_BodyActivationListener; }
#endif

    [[nodiscard]] bool getInit() const { return m_init; }

private:
    JPH::TempAllocatorImpl* m_TempAllocator;
    JPH::JobSystemThreadPool* m_JobSystem;

    BPLayerInterfaceImpl m_BroadPhaseLayerInterface;
    ObjectVsBroadPhaseLayerFilterImpl m_ObjectVsBroadphaseLayerFilter;
    ObjectLayerPairFilterImpl m_ObjectVsObjectLayerFilter;

    JPH::PhysicsSystem* m_PhysicsSystem;
    JPH::BodyInterface* m_BodyInterface;

#ifdef PHYSICS_DEBUG_LOG
    DebugContactListener m_ContactListener;
    DebugBodyActivationListener m_BodyActivationListener;
#endif

    bool m_init{false};
};
