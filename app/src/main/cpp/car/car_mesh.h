#pragma once
// car/car_mesh.h — Procedural car geometry builder
#include "../renderer/mesh_renderer.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace arracing {

// Builds car body mesh procedurally (box with chamfer + roof extrusion)
class CarMesh {
public:
    CarMesh() = default;

    // Generate and upload car body geometry
    void Build(float half_w, float half_h, float half_l,
               float roof_h = 0.18f, float chamfer = 0.04f);

    [[nodiscard]] const GpuMesh& Mesh() const { return mesh_; }
    [[nodiscard]] bool IsReady() const { return mesh_.IsReady(); }

private:
    GpuMesh mesh_;

    void AddChamferedBox(std::vector<Vertex>& verts,
                         std::vector<uint32_t>& indices,
                         float w, float h, float l, float chamfer);

    void AddQuad(std::vector<Vertex>& verts,
                 std::vector<uint32_t>& indices,
                 const glm::vec3& p0, const glm::vec3& p1,
                 const glm::vec3& p2, const glm::vec3& p3,
                 const glm::vec3& normal);
};

// Builds wheel/tyre geometry (cylinder with rounded edges)
class WheelMesh {
public:
    WheelMesh() = default;

    void Build(float radius, float width, int segments = 20);

    [[nodiscard]] const GpuMesh& Mesh() const { return mesh_; }
    [[nodiscard]] bool IsReady() const { return mesh_.IsReady(); }

private:
    GpuMesh mesh_;
};

} // namespace arracing
