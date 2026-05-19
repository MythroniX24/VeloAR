// renderer/plane_renderer.cc — AR plane visualization
#include "plane_renderer.h"
#include "shader_manager.h"
#include "../core/logger.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstring>

namespace arracing {

static const char* kPlaneVert = R"(#version 300 es
layout(location = 0) in vec3 a_Pos;
uniform mat4 u_MVP;
void main() {
    gl_Position = u_MVP * vec4(a_Pos, 1.0);
}
)";

static const char* kPlaneFrag = R"(#version 300 es
precision mediump float;
uniform vec4 u_Color;
out vec4 fragColor;
void main() {
    fragColor = u_Color;
}
)";

void PlaneRenderer::Init(AssetManager& /*assets*/) {
    ShaderManager sm;
    program_ = sm.CreateProgram(kPlaneVert, kPlaneFrag, "plane");

    uni_mvp_   = glGetUniformLocation(program_, "u_MVP");
    uni_color_ = glGetUniformLocation(program_, "u_Color");
    attr_pos_  = glGetAttribLocation(program_,  "a_Pos");

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_verts_);
    glGenBuffers(1, &vbo_indices_);

    initialized_ = true;
    LOGI("PlaneRenderer: Initialized");
}

void PlaneRenderer::UpdateMesh(const ArSession* session, const ArPlane* plane) {
    // Get polygon count from ARCore
    int32_t polygon_size = 0;
    ArPlane_getPolygonSize(session, plane, &polygon_size);
    if (polygon_size < 3) return;

    // Get raw polygon vertices (x, z pairs in local plane space)
    std::vector<float> raw(polygon_size * 2);
    ArPlane_getPolygon(session, plane, raw.data());

    // Get plane pose (converts local → world)
    ArPose* pose = nullptr;
    ArPose_create(session, nullptr, &pose);
    ArPlane_getCenterPose(session, plane, pose);

    float pose_raw[7];
    ArPose_getPoseRaw(session, pose, pose_raw);
    ArPose_destroy(pose);

    // Reconstruct model matrix from ARCore pose (quaternion + translation)
    // ARCore pose: [qx, qy, qz, qw, tx, ty, tz]
    glm::quat q(pose_raw[3], pose_raw[0], pose_raw[1], pose_raw[2]);
    glm::vec3 t(pose_raw[4], pose_raw[5], pose_raw[6]);
    model_mat_ = glm::translate(glm::mat4(1.0f), t) * glm::mat4_cast(q);

    // Build vertices in local plane space (y=0 for horizontal plane)
    vertices_.resize(polygon_size);
    for (int i = 0; i < polygon_size; ++i) {
        vertices_[i] = glm::vec3(raw[i*2], 0.0f, raw[i*2+1]);
    }

    // Fan triangulation from center
    indices_.clear();
    for (int i = 1; i < polygon_size - 1; ++i) {
        indices_.push_back(0);
        indices_.push_back(static_cast<uint16_t>(i));
        indices_.push_back(static_cast<uint16_t>(i + 1));
    }
}

void PlaneRenderer::DrawPlane(const glm::mat4& proj, const glm::mat4& view,
                              const ArSession* session, const ArPlane* plane) {
    if (!initialized_) return;

    UpdateMesh(session, plane);
    if (vertices_.empty() || indices_.empty()) return;

    // Upload geometry
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_verts_);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices_.size() * sizeof(glm::vec3),
                 vertices_.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(attr_pos_);
    glVertexAttribPointer(attr_pos_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_indices_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices_.size() * sizeof(uint16_t),
                 indices_.data(), GL_DYNAMIC_DRAW);

    glUseProgram(program_);

    glm::mat4 mvp = proj * view * model_mat_;
    glUniformMatrix4fv(uni_mvp_, 1, GL_FALSE, glm::value_ptr(mvp));

    // Semi-transparent cyan grid color
    glUniform4f(uni_color_, 0.0f, 1.0f, 0.9f, 0.25f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices_.size()),
                   GL_UNSIGNED_SHORT, nullptr);

    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glBindVertexArray(0);
    glUseProgram(0);
    CHECK_GL_ERROR("PlaneRenderer::DrawPlane");
}

PlaneRenderer::~PlaneRenderer() {
    if (program_)     glDeleteProgram(program_);
    if (vao_)         glDeleteVertexArrays(1, &vao_);
    if (vbo_verts_)   glDeleteBuffers(1, &vbo_verts_);
    if (vbo_indices_) glDeleteBuffers(1, &vbo_indices_);
}

} // namespace arracing
