// renderer/shadow_map.cc — Depth-only shadow pass
#include "shadow_map.h"
#include "shader_manager.h"
#include "../core/logger.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace arracing {

static const char* kDepthVert = R"(#version 300 es
layout(location = 0) in vec3 a_Pos;
uniform mat4 u_LightMVP;
void main() {
    gl_Position = u_LightMVP * vec4(a_Pos, 1.0);
}
)";

static const char* kDepthFrag = R"(#version 300 es
void main() {}
)";

bool ShadowMap::Init() {
    // Compile depth-only shader
    ShaderManager sm;
    depth_prog_ = sm.CreateProgram(kDepthVert, kDepthFrag, "shadow_depth");
    if (!depth_prog_) return false;

    // Create depth texture
    glGenTextures(1, &depth_tex_);
    glBindTexture(GL_TEXTURE_2D, depth_tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
                 kShadowRes, kShadowRes, 0,
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create FBO
    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, depth_tex_, 0);

    // No color attachment needed for depth-only pass
    const GLenum none = GL_NONE;
    glDrawBuffers(1, &none);
    glReadBuffer(GL_NONE);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("ShadowMap: FBO incomplete (status=0x%04x)", status);
        return false;
    }

    LOGI("ShadowMap: Initialized (%dx%d)", kShadowRes, kShadowRes);
    return true;
}

void ShadowMap::BeginShadowPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0, 0, kShadowRes, kShadowRes);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT); // Reduce peter-panning
}

void ShadowMap::EndShadowPass(int screen_w, int screen_h) {
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screen_w, screen_h);
}

glm::mat4 ShadowMap::LightSpaceMatrix(const glm::vec3& light_dir,
                                       const glm::vec3& scene_center) const {
    // Orthographic projection from light looking at scene center
    glm::vec3 light_pos = scene_center - glm::normalize(light_dir) * 5.0f;
    glm::mat4 light_view = glm::lookAt(light_pos, scene_center, glm::vec3(0,1,0));
    glm::mat4 light_proj = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 20.0f);
    return light_proj * light_view;
}

ShadowMap::~ShadowMap() {
    if (fbo_)       glDeleteFramebuffers(1, &fbo_);
    if (depth_tex_) glDeleteTextures(1, &depth_tex_);
    if (depth_prog_)glDeleteProgram(depth_prog_);
}

} // namespace arracing
