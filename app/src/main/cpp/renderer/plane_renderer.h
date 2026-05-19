#pragma once
// renderer/plane_renderer.h — Draws detected ARCore horizontal planes as grid overlay
#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <vector>
#include "../arcore_c_api.h"
#include "../core/asset_manager.h"

namespace arracing {

class PlaneRenderer {
public:
    PlaneRenderer() = default;
    ~PlaneRenderer();

    void Init(AssetManager& assets);

    // Draw all detected planes for visual feedback before car is placed
    void DrawPlane(const glm::mat4& projection, const glm::mat4& view,
                   const ArSession* session, const ArPlane* plane);

private:
    void UpdateMesh(const ArSession* session, const ArPlane* plane);

    std::vector<glm::vec3> vertices_;
    std::vector<uint16_t>  indices_;
    glm::mat4              model_mat_{1.0f};

    GLuint program_       = 0;
    GLuint vbo_verts_     = 0;
    GLuint vbo_indices_   = 0;
    GLuint vao_           = 0;

    GLint uni_mvp_        = -1;
    GLint uni_color_      = -1;
    
    GLint attr_pos_       = -1;

    bool   initialized_   = false;
};

} // namespace arracing
