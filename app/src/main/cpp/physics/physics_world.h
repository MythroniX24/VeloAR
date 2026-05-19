#pragma once
// physics/physics_world.h — Bullet Physics world management
#include <btBulletDynamicsCommon.h>
#include <memory>
#include <glm/glm.hpp>

namespace arracing {

// RAII wrapper for Bullet physics world
class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld();

    // Initialize world with gravity. Origin is AR world origin.
    bool Init(const glm::vec3& gravity = glm::vec3(0.0f, -9.81f, 0.0f));

    // Step simulation by delta seconds (handles sub-stepping internally)
    void Step(float delta_seconds);

    // Add/remove rigid bodies
    void AddRigidBody(btRigidBody* body);
    void RemoveRigidBody(btRigidBody* body);

    // Add action interface (btRaycastVehicle implements this)
    void AddVehicle(btActionInterface* vehicle);
    void RemoveVehicle(btActionInterface* vehicle);

    // Create static infinite ground plane (for when AR tracking is lost)
    btRigidBody* CreateGroundPlane(float y_level);

    // Create box rigid body
    btRigidBody* CreateBox(const glm::vec3& half_extents,
                           const glm::vec3& position,
                           float mass);

    [[nodiscard]] btDiscreteDynamicsWorld* World() const { return world_.get(); }

    // Set world origin offset (AR anchor position)
    void SetWorldOrigin(const glm::vec3& origin) { world_origin_ = origin; }
    [[nodiscard]] glm::vec3 WorldOrigin() const { return world_origin_; }

    // Utility: GLM ↔ Bullet conversion
    static btVector3  ToBullet(const glm::vec3& v) { return {v.x, v.y, v.z}; }
    static glm::vec3  ToGlm(const btVector3& v)    { return {v.x(), v.y(), v.z()}; }
    static glm::mat4  ToGlmMat(const btTransform& t);

private:
    std::unique_ptr<btDefaultCollisionConfiguration>     collision_config_;
    std::unique_ptr<btCollisionDispatcher>               dispatcher_;
    std::unique_ptr<btDbvtBroadphase>                    broadphase_;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver_;
    std::unique_ptr<btDiscreteDynamicsWorld>             world_;

    glm::vec3 world_origin_{0.0f};
    btRigidBody* ground_body_ = nullptr;
};

} // namespace arracing
