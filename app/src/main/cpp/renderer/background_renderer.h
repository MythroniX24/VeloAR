#pragma once
// renderer/background_renderer.h — ARCore camera passthrough renderer
// Renders the live camera image as fullscreen background before 3D scene
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>  // GL_TEXTURE_EXTERNAL_OES
#include "../arcore_c_api.h"
#include "../core/asset_manager.h"

namespace arracing {

class BackgroundRenderer {
public:
    BackgroundRenderer() = default;
    ~BackgroundRenderer();

    // Initialize GL state - call once on GL thread after surface created
    void Init(AssetManager& assets);

    // Draw camera frame - call every frame before 3D scene
    void Draw(const ArSession* session, const ArFrame* frame);

    // Returns the OES texture ID for ARCore to write camera data into
    [[nodiscard]] GLuint CameraTextureId() const { return camera_tex_id_; }

    [[nodiscard]] bool IsInitialized() const { return initialized_; }

private:
    static constexpr int kNumVerts = 4;

    // Fullscreen quad vertices in clip space
    static constexpr GLfloat kQuadVerts[] = {
        -1.0f, -1.0f,
        +1.0f, -1.0f,
        -1.0f, +1.0f,
        +1.0f, +1.0f,
    };

    GLuint camera_tex_id_     = 0;   // GL_TEXTURE_EXTERNAL_OES
    GLuint program_           = 0;
    GLuint vbo_position_      = 0;
    GLuint vbo_texcoord_      = 0;

    GLint  attr_position_     = -1;
    GLint  attr_texcoord_     = -1;
    GLint  uni_texture_       = -1;

    float  transformed_uvs_[kNumVerts * 2] = {};
    bool   uvs_initialized_   = false;
    bool   initialized_       = false;
};

} // namespace arracing
