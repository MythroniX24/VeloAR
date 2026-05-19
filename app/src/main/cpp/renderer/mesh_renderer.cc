// renderer/mesh_renderer.cc
#include "mesh_renderer.h"
#include "../core/logger.h"
#include <glm/gtc/type_ptr.hpp>

namespace arracing {

// ─── GpuMesh ──────────────────────────────────────────────────────────────

void GpuMesh::Upload(const std::vector<Vertex>& verts,
                     const std::vector<uint32_t>& indices) {
    Destroy(); // Clean up previous if any

    index_count_ = static_cast<uint32_t>(indices.size());

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // VBO for interleaved Vertex data
    glGenBuffers(1, &vbo_verts_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_verts_);
    glBufferData(GL_ARRAY_BUFFER,
                 verts.size() * sizeof(Vertex),
                 verts.data(), GL_STATIC_DRAW);

    // Position: location 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, position)));

    // Normal: location 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, normal)));

    // UV: location 2
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, uv)));

    // IBO
    glGenBuffers(1, &vbo_indices_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_indices_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(uint32_t),
                 indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    CHECK_GL_ERROR("GpuMesh::Upload");
    LOGI("GpuMesh: Uploaded %zu verts, %zu indices", verts.size(), indices.size());
}

void GpuMesh::Draw() const {
    if (!vao_) return;
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void GpuMesh::Destroy() {
    if (vbo_indices_) { glDeleteBuffers(1, &vbo_indices_); vbo_indices_ = 0; }
    if (vbo_verts_)   { glDeleteBuffers(1, &vbo_verts_);   vbo_verts_   = 0; }
    if (vao_)         { glDeleteVertexArrays(1, &vao_);    vao_         = 0; }
    index_count_ = 0;
}

GpuMesh::~GpuMesh() { Destroy(); }

// ─── MeshRenderer ─────────────────────────────────────────────────────────

void MeshRenderer::Init(GLuint program) {
    program_     = program;
    u_mvp_       = glGetUniformLocation(program, "u_MVP");
    u_model_     = glGetUniformLocation(program, "u_Model");
    u_albedo_    = glGetUniformLocation(program, "u_Albedo");
    u_roughness_ = glGetUniformLocation(program, "u_Roughness");
    u_metallic_  = glGetUniformLocation(program, "u_Metallic");
    u_albedo_tex_= glGetUniformLocation(program, "u_AlbedoTex");
    u_has_tex_   = glGetUniformLocation(program, "u_HasTexture");
    u_light_dir_ = glGetUniformLocation(program, "u_LightDir");
    u_cam_pos_   = glGetUniformLocation(program, "u_CamPos");
}

void MeshRenderer::Draw(const GpuMesh& mesh,
                        const Material& mat,
                        const glm::mat4& mvp,
                        const glm::mat4& model,
                        const glm::vec3& light_dir,
                        const glm::vec3& cam_pos) {
    if (!mesh.IsReady() || !program_) return;

    glUseProgram(program_);

    // Upload matrices
    glUniformMatrix4fv(u_mvp_,   1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(u_model_, 1, GL_FALSE, glm::value_ptr(model));

    // Material
    glUniform3fv(u_albedo_,    1, glm::value_ptr(mat.albedo));
    glUniform1f(u_roughness_,  mat.roughness);
    glUniform1f(u_metallic_,   mat.metallic);

    // Texture (optional)
    bool has_tex = (mat.albedo_tex != 0);
    glUniform1i(u_has_tex_, has_tex ? 1 : 0);
    if (has_tex) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mat.albedo_tex);
        glUniform1i(u_albedo_tex_, 0);
    }

    // Lighting
    glUniform3fv(u_light_dir_, 1, glm::value_ptr(light_dir));
    glUniform3fv(u_cam_pos_,   1, glm::value_ptr(cam_pos));

    mesh.Draw();

    glUseProgram(0);
    CHECK_GL_ERROR("MeshRenderer::Draw");
}

MeshRenderer::~MeshRenderer() = default;

} // namespace arracing
