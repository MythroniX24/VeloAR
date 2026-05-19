// car/car_mesh.cc — Procedural car geometry
#include "car_mesh.h"
#include "../core/logger.h"
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace arracing {

// ─── CarMesh ───────────────────────────────────────────────────────────────

void CarMesh::AddQuad(std::vector<Vertex>& verts, std::vector<uint32_t>& indices,
                      const glm::vec3& p0, const glm::vec3& p1,
                      const glm::vec3& p2, const glm::vec3& p3,
                      const glm::vec3& normal) {
    uint32_t base = static_cast<uint32_t>(verts.size());
    auto uv = [](int i) -> glm::vec2 {
        const glm::vec2 uvs[4] = {{0,0},{1,0},{1,1},{0,1}};
        return uvs[i];
    };
    verts.push_back({p0, normal, uv(0)});
    verts.push_back({p1, normal, uv(1)});
    verts.push_back({p2, normal, uv(2)});
    verts.push_back({p3, normal, uv(3)});
    indices.push_back(base + 0); indices.push_back(base + 1); indices.push_back(base + 2);
    indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 3);
}

void CarMesh::AddChamferedBox(std::vector<Vertex>& verts,
                               std::vector<uint32_t>& indices,
                               float w, float h, float l, float /*chamfer*/) {
    // 6 faces of box
    glm::vec3 e(w, h, l);

    // Front (+Z)
    AddQuad(verts, indices,
        {-e.x, -e.y, e.z}, {e.x, -e.y, e.z},
        {e.x,  e.y,  e.z}, {-e.x, e.y, e.z},
        {0, 0, 1});
    // Back (-Z)
    AddQuad(verts, indices,
        {e.x, -e.y, -e.z}, {-e.x, -e.y, -e.z},
        {-e.x, e.y, -e.z}, {e.x, e.y, -e.z},
        {0, 0, -1});
    // Right (+X)
    AddQuad(verts, indices,
        {e.x, -e.y, e.z}, {e.x, -e.y, -e.z},
        {e.x,  e.y,-e.z}, {e.x,  e.y, e.z},
        {1, 0, 0});
    // Left (-X)
    AddQuad(verts, indices,
        {-e.x, -e.y, -e.z}, {-e.x, -e.y, e.z},
        {-e.x,  e.y, e.z},  {-e.x,  e.y,-e.z},
        {-1, 0, 0});
    // Top (+Y)
    AddQuad(verts, indices,
        {-e.x, e.y, e.z}, {e.x, e.y, e.z},
        {e.x,  e.y,-e.z}, {-e.x, e.y,-e.z},
        {0, 1, 0});
    // Bottom (-Y)
    AddQuad(verts, indices,
        {-e.x,-e.y,-e.z}, {e.x, -e.y,-e.z},
        {e.x, -e.y, e.z}, {-e.x,-e.y, e.z},
        {0,-1, 0});
}

void CarMesh::Build(float hw, float hh, float hl, float roof_h, float chamfer) {
    std::vector<Vertex>   verts;
    std::vector<uint32_t> indices;
    verts.reserve(256);
    indices.reserve(512);

    // Main chassis body (lower box)
    AddChamferedBox(verts, indices, hw, hh, hl, chamfer);

    // Roof / cabin (smaller upper box)
    float roof_w = hw * 0.75f;
    float roof_l = hl * 0.65f;
    float roof_base_y = hh;

    auto add_offset_box = [&](float ow, float oh, float ol, float ox, float oy, float oz) {
        auto save = verts.size();
        (void)save;
        // Translate all new verts by offset after adding
        size_t before = verts.size();
        AddChamferedBox(verts, indices, ow, oh, ol, chamfer * 0.5f);
        for (size_t i = before; i < verts.size(); ++i) {
            verts[i].position += glm::vec3(ox, oy, oz);
        }
    };

    add_offset_box(roof_w, roof_h, roof_l,
                   0.0f, roof_base_y + roof_h * 0.5f, hl * 0.05f);

    // Spoiler (rear decorative fin)
    float spoiler_h = hh * 0.6f;
    float spoiler_w = hw * 0.85f;
    float spoiler_t = 0.03f;
    add_offset_box(spoiler_w, spoiler_h, spoiler_t,
                   0.0f, hh + spoiler_h, -(hl - spoiler_t));

    mesh_.Upload(verts, indices);
    LOGI("CarMesh: Built %zu verts, %zu indices", verts.size(), indices.size());
}

// ─── WheelMesh ─────────────────────────────────────────────────────────────

void WheelMesh::Build(float radius, float width, int segments) {
    std::vector<Vertex>   verts;
    std::vector<uint32_t> indices;

    const float pi   = glm::pi<float>();
    const float half = width * 0.5f;

    // Generate cylinder around X axis (axle direction)
    for (int i = 0; i <= segments; ++i) {
        float angle = (static_cast<float>(i) / segments) * 2.0f * pi;
        float y = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;
        glm::vec3 normal(0.0f, std::cos(angle), std::sin(angle));
        glm::vec2 uv_coord(static_cast<float>(i) / segments, 0.0f);

        // Left side disc
        verts.push_back({{-half, y, z}, {-1, 0, 0}, uv_coord});
        // Right side disc
        verts.push_back({{ half, y, z}, { 1, 0, 0}, uv_coord});
        // Tread surface
        verts.push_back({{-half, y, z}, normal, uv_coord});
        verts.push_back({{ half, y, z}, normal, uv_coord});
    }

    // Build strip faces
    for (int i = 0; i < segments; ++i) {
        uint32_t base = static_cast<uint32_t>(i * 4);
        // Tread (outer surface)
        indices.push_back(base + 2); indices.push_back(base + 3); indices.push_back(base + 6);
        indices.push_back(base + 3); indices.push_back(base + 7); indices.push_back(base + 6);
    }

    // Left disc cap
    uint32_t center_l = static_cast<uint32_t>(verts.size());
    verts.push_back({{-half, 0, 0}, {-1,0,0}, {0.5f, 0.5f}});
    for (int i = 0; i < segments; ++i) {
        uint32_t a = static_cast<uint32_t>(i * 4);
        uint32_t b = static_cast<uint32_t>((i + 1) * 4);
        indices.push_back(center_l); indices.push_back(b); indices.push_back(a);
    }

    // Right disc cap
    uint32_t center_r = static_cast<uint32_t>(verts.size());
    verts.push_back({{ half, 0, 0}, {1,0,0}, {0.5f, 0.5f}});
    for (int i = 0; i < segments; ++i) {
        uint32_t a = static_cast<uint32_t>(i * 4 + 1);
        uint32_t b = static_cast<uint32_t>((i + 1) * 4 + 1);
        indices.push_back(center_r); indices.push_back(a); indices.push_back(b);
    }

    mesh_.Upload(verts, indices);
    LOGI("WheelMesh: Built %zu verts, %zu indices", verts.size(), indices.size());
}

} // namespace arracing
