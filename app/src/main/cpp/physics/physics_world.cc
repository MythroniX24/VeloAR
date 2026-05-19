// physics/physics_world.cc — Bullet physics world
#include "physics_world.h"
#include "../core/logger.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>

namespace arracing {

PhysicsWorld::PhysicsWorld() = default;

bool PhysicsWorld::Init(const glm::vec3& gravity) {
    // Standard Bullet pipeline: config → dispatcher → broadphase → solver → world
    collision_config_ = std::make_unique<btDefaultCollisionConfiguration>();
    dispatcher_       = std::make_unique<btCollisionDispatcher>(collision_config_.get());
    broadphase_       = std::make_unique<btDbvtBroadphase>();
    solver_           = std::make_unique<btSequentialImpulseConstraintSolver>();

    world_ = std::make_unique<btDiscreteDynamicsWorld>(
        dispatcher_.get(), broadphase_.get(),
        solver_.get(), collision_config_.get());

    world_->setGravity(ToBullet(gravity));

    LOGI("PhysicsWorld: Initialized gravity=(%.2f,%.2f,%.2f)",
         gravity.x, gravity.y, gravity.z);
    return true;
}

void PhysicsWorld::Step(float delta_seconds) {
    if (!world_) return;

    // Bullet sub-steps at 240Hz internally for stability (capped at 10 sub-steps)
    constexpr float kFixedStep = 1.0f / 240.0f;
    world_->stepSimulation(delta_seconds, 10, kFixedStep);
}

void PhysicsWorld::AddRigidBody(btRigidBody* body) {
    if (world_ && body) world_->addRigidBody(body);
}

void PhysicsWorld::RemoveRigidBody(btRigidBody* body) {
    if (world_ && body) world_->removeRigidBody(body);
}

void PhysicsWorld::AddVehicle(btActionInterface* vehicle) {
    if (world_ && vehicle) world_->addAction(vehicle);
}

void PhysicsWorld::RemoveVehicle(btActionInterface* vehicle) {
    if (world_ && vehicle) world_->removeAction(vehicle);
}

btRigidBody* PhysicsWorld::CreateGroundPlane(float y_level) {
    if (ground_body_) {
        RemoveRigidBody(ground_body_);
        delete ground_body_->getCollisionShape();
        delete ground_body_;
        ground_body_ = nullptr;
    }

    // Static infinite plane at y_level
    auto* shape = new btStaticPlaneShape(btVector3(0, 1, 0), y_level);
    btTransform xform;
    xform.setIdentity();

    auto* motion = new btDefaultMotionState(xform);
    btRigidBody::btRigidBodyConstructionInfo info(0.0f, motion, shape);
    info.m_restitution = 0.3f;
    info.m_friction    = 0.8f;

    ground_body_ = new btRigidBody(info);
    AddRigidBody(ground_body_);

    LOGI("PhysicsWorld: Ground plane created at y=%.3f", y_level);
    return ground_body_;
}

btRigidBody* PhysicsWorld::CreateBox(const glm::vec3& half_extents,
                                     const glm::vec3& position,
                                     float mass) {
    auto* shape = new btBoxShape(ToBullet(half_extents));

    btTransform xform;
    xform.setIdentity();
    xform.setOrigin(ToBullet(position));

    btVector3 inertia(0, 0, 0);
    if (mass > 0.0f) shape->calculateLocalInertia(mass, inertia);

    auto* motion = new btDefaultMotionState(xform);
    btRigidBody::btRigidBodyConstructionInfo info(mass, motion, shape, inertia);
    auto* body = new btRigidBody(info);

    AddRigidBody(body);
    return body;
}

glm::mat4 PhysicsWorld::ToGlmMat(const btTransform& t) {
    float m[16];
    t.getOpenGLMatrix(m);
    return glm::make_mat4(m);
}

PhysicsWorld::~PhysicsWorld() {
    // Remove and delete ground plane
    if (ground_body_ && world_) {
        world_->removeRigidBody(ground_body_);
        delete ground_body_->getMotionState();
        delete ground_body_->getCollisionShape();
        delete ground_body_;
    }
    // world_, solver_, broadphase_, dispatcher_, collision_config_ auto-destroyed
    LOGI("PhysicsWorld: Destroyed");
}

} // namespace arracing
