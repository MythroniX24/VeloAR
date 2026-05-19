#pragma once
// renderer/mesh_renderer.h — Generic VAO/VBO mesh renderer
#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace arracing {

// Vertex with position, normal, UV
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

// Per-draw material uniforms
struct Material {
    glm::vec3 albedo       = glm::vec3(0.8f, 0.8f, 0.8f);
    float     roughness    = 0.5f;
    float     metallic     = 0.0f;
    GLuint    albedo_tex   = 0;
};

// Simple mesh uploaded to GPU
class GpuMesh {
public:
    GpuMesh() = default;
    ~GpuMesh();

    // Upload to GPU - call on GL thread
    void Upload(const std::vector<Vertex>& verts,
                const std::vector<uint32_t>& indices);

    void Draw() const;
    void Destroy();

    [[nodiscard]] bool IsReady() const { return vao_ != 0; }
    [[nodiscard]] uint32_t IndexCount() const { return index_count_; }

private:
    GLuint vao_         = 0;
    GLuint vbo_verts_   = 0;
    GLuint vbo_indices_ = 0;
    uint32_t index_count_ = 0;
};

// Renderer that handles a set of GpuMesh + Material + transform
class MeshRenderer {
public:
    MeshRenderer() = default;
    ~MeshRenderer();

    // Initialize with GLSL program (created by ShaderManager externally)
    void Init(GLuint program);

    // Render one mesh with given MVP, model matrix (for normals), and material
    void Draw(const GpuMesh& mesh,
              const Material& mat,
              const glm::mat4& mvp,
              const glm::mat4& model,
              const glm::vec3& light_dir,
              const glm::vec3& cam_pos);

private:
    GLuint program_ = 0;

    // Uniform locations
    GLint u_mvp_         = -1;
    GLint u_model_       = -1;
    GLint u_albedo_      = -1;
    GLint u_roughness_   = -1;
    GLint u_metallic_    = -1;
    GLint u_albedo_tex_  = -1;
    GLint u_has_tex_     = -1;
    GLint u_light_dir_   = -1;
    GLint u_cam_pos_     = -1;
};

} // namespace arracing
