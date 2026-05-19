#pragma once
// car/car_entity.h — The complete car: physics + rendering + state
#include "car_mesh.h"
#include "../physics/vehicle_physics.h"
#include "../renderer/mesh_renderer.h"
#include "../core/asset_manager.h"
#include <glm/glm.hpp>
#include <memory>

namespace arracing {

class CarEntity {
public:
    CarEntity() = default;

    // Initialize meshes, materials, and physics in one call
    bool Init(PhysicsWorld& world, const glm::vec3& spawn_pos,
              AssetManager& assets, MeshRenderer& renderer);

    // Update physics (call at physics rate)
    void UpdatePhysics(const InputState& input, float dt);

    // Render chassis + 4 wheels
    void Render(MeshRenderer& renderer,
                const glm::mat4& view_proj,
                const glm::vec3& light_dir,
                const glm::vec3& cam_pos,
                const glm::mat4& anchor_mat);

    // Reset position to anchor origin
    void ResetToAnchor(const glm::mat4& anchor_mat);

    [[nodiscard]] const VehicleState& State() const {
        return vehicle_.State();
    }

    [[nodiscard]] VehiclePhysics& Physics() { return vehicle_; }

private:
    VehiclePhysics vehicle_;
    CarMesh        car_mesh_;
    WheelMesh      wheel_mesh_;

    Material chassis_mat_;
    Material wheel_mat_;

    GLuint   car_program_  = 0;
};

} // namespace arracing
