// physics/vehicle_physics.cc — btRaycastVehicle driving physics
#include "vehicle_physics.h"
#include "../core/logger.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>

namespace arracing {

bool VehiclePhysics::Init(PhysicsWorld& pw, const glm::vec3& spawn_pos,
                           const VehicleTuning& tuning) {
    physics_world_ = &pw;
    tuning_        = tuning;

    // ── Chassis rigid body ────────────────────────────────────────────────
    chassis_shape_ = new btBoxShape(
        btVector3(tuning_.chassis_w, tuning_.chassis_h, tuning_.chassis_l));

    btTransform xform;
    xform.setIdentity();
    // Spawn slightly above plane to avoid initial collision tunnelling
    xform.setOrigin(btVector3(spawn_pos.x, spawn_pos.y + 0.5f, spawn_pos.z));

    btVector3 inertia(0, 0, 0);
    chassis_shape_->calculateLocalInertia(tuning_.mass, inertia);

    auto* motion_state = new btDefaultMotionState(xform);
    btRigidBody::btRigidBodyConstructionInfo chassis_info(
        tuning_.mass, motion_state, chassis_shape_, inertia);
    chassis_info.m_restitution = 0.05f;
    chassis_info.m_friction    = 0.5f;

    chassis_body_ = new btRigidBody(chassis_info);
    // Prevent chassis from sleeping (stays responsive to input)
    chassis_body_->setActivationState(DISABLE_DEACTIVATION);
    // Linear damping to prevent sliding; angular damping for stability
    chassis_body_->setDamping(0.02f, 0.05f);

    pw.AddRigidBody(chassis_body_);

    // ── btRaycastVehicle ─────────────────────────────────────────────────
    raycaster_ = std::make_unique<btDefaultVehicleRaycaster>(pw.World());

    btRaycastVehicle::btVehicleTuning bt_tuning;
    bt_tuning.m_suspensionStiffness    = tuning_.suspension_stiff;
    bt_tuning.m_suspensionDamping      = tuning_.suspension_damp;
    bt_tuning.m_suspensionCompression  = tuning_.suspension_comp;
    bt_tuning.m_frictionSlip           = tuning_.friction;
    bt_tuning.m_maxSuspensionTravelCm  = 50.0f;
    bt_tuning.m_maxSuspensionForce     = 10000.0f;

    vehicle_ = std::make_unique<btRaycastVehicle>(
        bt_tuning, chassis_body_, raycaster_.get());

    // Right = +X, Up = +Y, Forward = +Z (ARCore convention)
    vehicle_->setCoordinateSystem(0, 1, 2);

    pw.AddVehicle(vehicle_.get());

    AddWheels();

    LOGI("VehiclePhysics: Initialized at (%.2f, %.2f, %.2f)",
         spawn_pos.x, spawn_pos.y, spawn_pos.z);
    return true;
}

void VehiclePhysics::AddWheels() {
    btRaycastVehicle::btVehicleTuning bt_tuning;
    bt_tuning.m_suspensionStiffness   = tuning_.suspension_stiff;
    bt_tuning.m_suspensionDamping     = tuning_.suspension_damp;
    bt_tuning.m_suspensionCompression = tuning_.suspension_comp;
    bt_tuning.m_frictionSlip          = tuning_.friction;
    bt_tuning.m_maxSuspensionTravelCm = 50.0f;
    bt_tuning.m_maxSuspensionForce    = 10000.0f;

    // Wheel positions relative to chassis center
    // Layout: [FL, FR, RL, RR]
    const float fw = tuning_.chassis_l  * 0.75f; // front axle offset
    const float rw = tuning_.chassis_l  * 0.75f; // rear axle offset
    const float sw = tuning_.chassis_w  + 0.05f; // side offset

    struct WheelDef { btVector3 pos; bool is_front; };
    WheelDef defs[4] = {
        { btVector3(-sw,  0,  fw), true  }, // FL
        { btVector3( sw,  0,  fw), true  }, // FR
        { btVector3(-sw,  0, -rw), false }, // RL
        { btVector3( sw,  0, -rw), false }, // RR
    };

    for (auto& w : defs) {
        vehicle_->addWheel(
            w.pos,
            btVector3(0, -1, 0),            // wheel down direction
            btVector3(-1, 0, 0),            // axle direction (left)
            tuning_.suspension_rest,        // rest length
            tuning_.wheel_radius,
            bt_tuning,
            w.is_front                      // is front wheel (steers)
        );
    }

    // Per-wheel friction (rear slightly lower for drift)
    for (int i = 0; i < 4; ++i) {
        auto& wi = vehicle_->getWheelInfo(i);
        wi.m_rollInfluence = 0.1f;  // reduce roll
        if (i >= 2) wi.m_frictionSlip = tuning_.friction * 0.85f; // rear drift
    }
}

void VehiclePhysics::Update(const InputState& input, float dt) {
    if (!vehicle_) return;

    // ── Steering (smooth interpolation) ──────────────────────────────────
    float target_steer = input.steer_x.load() * tuning_.max_steer;
    float steer_delta  = tuning_.steer_speed * dt;
    if (current_steer_ < target_steer)
        current_steer_ = std::min(current_steer_ + steer_delta, target_steer);
    else
        current_steer_ = std::max(current_steer_ - steer_delta, target_steer);

    vehicle_->setSteeringValue(-current_steer_, 0); // FL
    vehicle_->setSteeringValue(-current_steer_, 1); // FR

    // ── Speed clamping ────────────────────────────────────────────────────
    float speed_kmh = vehicle_->getCurrentSpeedKmHour();
    bool  max_speed = std::abs(speed_kmh) >= tuning_.max_speed_kmh;

    // ── Engine / Brake / Handbrake ────────────────────────────────────────
    bool throttle  = input.throttle.load();
    bool brake     = input.brake.load();
    bool handbrake = input.handbrake.load();

    // Drive rear wheels only (RWD)
    float engine = throttle && !max_speed ? tuning_.engine_force : 0.0f;
    float brk    = brake ? tuning_.brake_force : 0.0f;

    vehicle_->applyEngineForce(engine,  2); // RL
    vehicle_->applyEngineForce(engine,  3); // RR
    vehicle_->applyEngineForce(0.0f,    0); // FL freewheeling
    vehicle_->applyEngineForce(0.0f,    1); // FR freewheeling

    // Regular brake on all 4
    for (int i = 0; i < 4; ++i) {
        vehicle_->setBrake(brk, i);
    }

    // Handbrake: only on rear, higher force for drift
    if (handbrake) {
        vehicle_->setBrake(tuning_.brake_force * 4.0f, 2);
        vehicle_->setBrake(tuning_.brake_force * 4.0f, 3);
    }

    // ── Update live state ─────────────────────────────────────────────────
    state_.speed_kmh   = speed_kmh;
    state_.steer_angle = current_steer_;
    state_.airborne    = !vehicle_->getWheelInfo(0).m_raycastInfo.m_isInContact &&
                         !vehicle_->getWheelInfo(2).m_raycastInfo.m_isInContact;

    const btVector3& vel = chassis_body_->getLinearVelocity();
    state_.velocity    = PhysicsWorld::ToGlm(vel);
    state_.rpm         = std::abs(engine) / tuning_.engine_force * 7000.0f;

    btTransform chassis_xform;
    chassis_body_->getMotionState()->getWorldTransform(chassis_xform);
    state_.position = PhysicsWorld::ToGlm(chassis_xform.getOrigin());
}

glm::mat4 VehiclePhysics::ChassisTransform() const {
    if (!chassis_body_) return glm::mat4(1.0f);
    btTransform t;
    chassis_body_->getMotionState()->getWorldTransform(t);
    return PhysicsWorld::ToGlmMat(t);
}

glm::mat4 VehiclePhysics::WheelTransform(int wheel) const {
    if (!vehicle_ || wheel < 0 || wheel >= vehicle_->getNumWheels())
        return glm::mat4(1.0f);

    vehicle_->updateWheelTransform(wheel, true);
    const btTransform& wt = vehicle_->getWheelInfo(wheel).m_worldTransform;
    return PhysicsWorld::ToGlmMat(wt);
}

void VehiclePhysics::SetPosition(const glm::vec3& pos) {
    if (!chassis_body_) return;
    btTransform xform;
    xform.setIdentity();
    xform.setOrigin(btVector3(pos.x, pos.y + 0.5f, pos.z));
    chassis_body_->setWorldTransform(xform);
    chassis_body_->getMotionState()->setWorldTransform(xform);
    chassis_body_->setLinearVelocity(btVector3(0, 0, 0));
    chassis_body_->setAngularVelocity(btVector3(0, 0, 0));
}

VehiclePhysics::~VehiclePhysics() {
    if (physics_world_ && vehicle_)
        physics_world_->RemoveVehicle(vehicle_.get());
    if (physics_world_ && chassis_body_)
        physics_world_->RemoveRigidBody(chassis_body_);

    if (chassis_body_) {
        delete chassis_body_->getMotionState();
        delete chassis_body_;
        chassis_body_ = nullptr;
    }
    if (chassis_shape_) {
        delete chassis_shape_;
        chassis_shape_ = nullptr;
    }
    LOGI("VehiclePhysics: Destroyed");
}

} // namespace arracing
