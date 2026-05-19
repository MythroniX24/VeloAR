#pragma once
// physics/vehicle_physics.h — btRaycastVehicle based car physics
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <glm/glm.hpp>
#include <array>
#include <memory>
#include "../input/input_state.h"
#include "physics_world.h"

namespace arracing {

// Tunable vehicle parameters
struct VehicleTuning {
    float mass              = 800.0f;   // kg
    float chassis_w         = 0.9f;     // half-width
    float chassis_h         = 0.2f;     // half-height
    float chassis_l         = 1.8f;     // half-length
    float wheel_radius      = 0.25f;
    float wheel_width       = 0.2f;
    float suspension_rest   = 0.35f;    // rest length m
    float suspension_stiff  = 20.0f;
    float suspension_damp   = 2.3f;
    float suspension_comp   = 4.4f;
    float friction          = 1000.0f;
    float max_steer         = 0.5f;     // radians (~28°)
    float steer_speed       = 2.5f;     // rad/s interpolation
    float engine_force      = 2000.0f;
    float brake_force       = 100.0f;
    float max_speed_kmh     = 80.0f;
};

// Live vehicle state for HUD / rendering
struct VehicleState {
    float   speed_kmh   = 0.0f;
    float   steer_angle = 0.0f;
    bool    airborne    = false;
    float   rpm         = 0.0f;
    glm::vec3 position  = {0,0,0};
    glm::vec3 velocity  = {0,0,0};
};

class VehiclePhysics {
public:
    VehiclePhysics() = default;
    ~VehiclePhysics();

    // Spawn vehicle in physics world at given AR world position
    bool Init(PhysicsWorld& physics_world, const glm::vec3& spawn_pos,
              const VehicleTuning& tuning = VehicleTuning{});

    // Apply input and step vehicle logic (called at physics rate)
    void Update(const InputState& input, float dt);

    // Get chassis world transform for rendering
    [[nodiscard]] glm::mat4 ChassisTransform() const;

    // Get wheel transform (0=FL, 1=FR, 2=RL, 3=RR)
    [[nodiscard]] glm::mat4 WheelTransform(int wheel) const;

    [[nodiscard]] const VehicleState& State() const { return state_; }

    // Teleport car to new position (anchor changed)
    void SetPosition(const glm::vec3& pos);

    [[nodiscard]] btRaycastVehicle* Vehicle() const { return vehicle_.get(); }
    [[nodiscard]] btRigidBody*      Chassis() const { return chassis_body_; }

private:
    PhysicsWorld*                          physics_world_ = nullptr;
    btRigidBody*                           chassis_body_  = nullptr;
    std::unique_ptr<btVehicleRaycaster>    raycaster_;
    std::unique_ptr<btRaycastVehicle>      vehicle_;
    btBoxShape*                            chassis_shape_ = nullptr;

    VehicleTuning  tuning_;
    VehicleState   state_;
    float          current_steer_ = 0.0f;

    void AddWheels();
};

} // namespace arracing
