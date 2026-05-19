#pragma once
// renderer/shadow_map.h — Single directional shadow map
#include <GLES3/gl3.h>
#include <glm/glm.hpp>

namespace arracing {

class ShadowMap {
public:
    static constexpr int kShadowRes = 1024;

    ShadowMap() = default;
    ~ShadowMap();

    bool Init();

    // Begin shadow pass - bind FBO and render scene from light POV
    void BeginShadowPass();
    void EndShadowPass(int screen_w, int screen_h);

    // Compute orthographic light matrix for ground-level scene
    [[nodiscard]] glm::mat4 LightSpaceMatrix(const glm::vec3& light_dir,
                                              const glm::vec3& scene_center) const;

    [[nodiscard]] GLuint DepthTexture() const { return depth_tex_; }
    [[nodiscard]] GLuint ShadowProgram() const { return depth_prog_; }

private:
    GLuint fbo_       = 0;
    GLuint depth_tex_ = 0;
    GLuint depth_prog_= 0;
};

} // namespace arracing
