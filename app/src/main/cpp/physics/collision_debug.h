#pragma once
// physics/collision_debug.h — Bullet debug draw via OpenGL ES line rendering
#include <btBulletDynamicsCommon.h>
#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

namespace arracing {

class BulletDebugDraw : public btIDebugDraw {
public:
    BulletDebugDraw() = default;
    ~BulletDebugDraw() override;

    void Init();

    // Render all collected lines and clear buffer
    void Flush(const glm::mat4& view_proj);

    // btIDebugDraw interface
    void drawLine(const btVector3& from, const btVector3& to,
                  const btVector3& color) override;
    void drawContactPoint(const btVector3& /*pt*/, const btVector3& /*normal*/,
                          btScalar /*dist*/, int /*lifetime*/,
                          const btVector3& /*color*/) override {}
    void reportErrorWarning(const char* /*warn*/) override {}
    void draw3dText(const btVector3& /*loc*/, const char* /*text*/) override {}
    void setDebugMode(int mode) override { debug_mode_ = mode; }
    int  getDebugMode() const override   { return debug_mode_; }

private:
    struct LineVertex { float x, y, z, r, g, b; };
    std::vector<LineVertex> lines_;

    GLuint vao_     = 0;
    GLuint vbo_     = 0;
    GLuint program_ = 0;
    GLint  u_mvp_   = -1;
    int    debug_mode_ = DBG_DrawWireframe;
};

} // namespace arracing
