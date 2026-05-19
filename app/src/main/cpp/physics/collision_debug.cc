// physics/collision_debug.cc — Bullet physics wireframe debug renderer
#include "collision_debug.h"
#include "../renderer/shader_manager.h"
#include "../core/logger.h"

namespace arracing {

static const char* kDebugVert = R"(#version 300 es
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Color;
uniform mat4 u_MVP;
out vec3 v_Color;
void main() {
    gl_Position = u_MVP * vec4(a_Pos, 1.0);
    v_Color = a_Color;
}
)";
static const char* kDebugFrag = R"(#version 300 es
precision mediump float;
in  vec3 v_Color;
out vec4 fragColor;
void main() { fragColor = vec4(v_Color, 1.0); }
)";

void BulletDebugDraw::Init() {
    arracing::ShaderManager sm;
    program_ = sm.CreateProgram(kDebugVert, kDebugFrag, "bullet_debug");
    u_mvp_   = glGetUniformLocation(program_, "u_MVP");

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(LineVertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(LineVertex), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
}

void BulletDebugDraw::drawLine(const btVector3& from, const btVector3& to,
                                const btVector3& color) {
    lines_.push_back({from.x(), from.y(), from.z(), color.x(), color.y(), color.z()});
    lines_.push_back({to.x(),   to.y(),   to.z(),   color.x(), color.y(), color.z()});
}

void BulletDebugDraw::Flush(const glm::mat4& vp) {
    if (lines_.empty() || !program_) return;

    glUseProgram(program_);
    glUniformMatrix4fv(u_mvp_, 1, GL_FALSE, glm::value_ptr(vp));

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 lines_.size() * sizeof(LineVertex),
                 lines_.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines_.size()));
    glBindVertexArray(0);
    glUseProgram(0);

    lines_.clear();
}

BulletDebugDraw::~BulletDebugDraw() {
    if (vao_)     glDeleteVertexArrays(1, &vao_);
    if (vbo_)     glDeleteBuffers(1, &vbo_);
    if (program_) glDeleteProgram(program_);
}

} // namespace arracing
