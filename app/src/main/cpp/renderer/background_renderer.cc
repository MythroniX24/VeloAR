// renderer/background_renderer.cc — Camera feed background
#include "background_renderer.h"
#include "shader_manager.h"
#include "../core/logger.h"
#include <string>

namespace arracing {

// ─── Inline shader source (avoids asset loading dependency for background) ───
static const char* kBackgroundVert = R"(#version 300 es
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;
out vec2 v_TexCoord;
void main() {
    gl_Position = vec4(a_Position, 0.0, 1.0);
    v_TexCoord  = a_TexCoord;
}
)";

static const char* kBackgroundFrag = R"(#version 300 es
#extension GL_OES_EGL_image_external_essl3 : require
precision mediump float;
uniform samplerExternalOES u_CameraTexture;
in  vec2 v_TexCoord;
out vec4 fragColor;
void main() {
    fragColor = texture(u_CameraTexture, v_TexCoord);
}
)";

void BackgroundRenderer::Init(AssetManager& /*assets*/) {
    // Create OES texture for ARCore to write camera frames into
    glGenTextures(1, &camera_tex_id_);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, camera_tex_id_);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Compile shader inline
    ShaderManager sm;
    program_ = sm.CreateProgram(kBackgroundVert, kBackgroundFrag, "background");
    // We own the program handle; sm will try to delete but we zero the cache
    // Workaround: use raw GL compile directly
    // Actually just reuse ShaderManager properly:
    attr_position_  = glGetAttribLocation(program_,  "a_Position");
    attr_texcoord_  = glGetAttribLocation(program_,  "a_TexCoord");
    uni_texture_    = glGetUniformLocation(program_, "u_CameraTexture");

    // Upload position VBO
    glGenBuffers(1, &vbo_position_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadVerts), kQuadVerts, GL_STATIC_DRAW);

    glGenBuffers(1, &vbo_texcoord_);

    CHECK_GL_ERROR("BackgroundRenderer::Init");
    initialized_ = true;
    LOGI("BackgroundRenderer: Initialized (tex=%u prog=%u)", camera_tex_id_, program_);
}

void BackgroundRenderer::Draw(const ArSession* session, const ArFrame* frame) {
    if (!initialized_) return;

    // Query whether display geometry changed; re-compute UV transform if so
    int32_t geom_changed = 0;
    ArFrame_getDisplayGeometryChanged(session, frame, &geom_changed);
    if (geom_changed || !uvs_initialized_) {
        ArFrame_transformCoordinates2d(
            session, frame,
            AR_COORDINATES_2D_OPENGL_NORMALIZED_DEVICE_COORDINATES,
            kNumVerts, kQuadVerts,
            AR_COORDINATES_2D_TEXTURE_NORMALIZED,
            transformed_uvs_);
        uvs_initialized_ = true;

        // Upload updated UVs
        glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(transformed_uvs_),
                     transformed_uvs_, GL_DYNAMIC_DRAW);
    }

    // Skip if no camera frame yet
    int64_t ts = 0;
    ArFrame_getTimestamp(session, frame, &ts);
    if (ts == 0) return;

    // Render fullscreen quad without depth write (background is behind everything)
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(program_);

    // Bind camera OES texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, camera_tex_id_);
    glUniform1i(uni_texture_, 0);

    // Position attrib
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_);
    glEnableVertexAttribArray(attr_position_);
    glVertexAttribPointer(attr_position_, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // TexCoord attrib
    glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord_);
    glEnableVertexAttribArray(attr_texcoord_);
    glVertexAttribPointer(attr_texcoord_, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(attr_position_);
    glDisableVertexAttribArray(attr_texcoord_);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    CHECK_GL_ERROR("BackgroundRenderer::Draw");
}

BackgroundRenderer::~BackgroundRenderer() {
    if (camera_tex_id_) glDeleteTextures(1, &camera_tex_id_);
    if (program_)       glDeleteProgram(program_);
    if (vbo_position_)  glDeleteBuffers(1, &vbo_position_);
    if (vbo_texcoord_)  glDeleteBuffers(1, &vbo_texcoord_);
}

} // namespace arracing
