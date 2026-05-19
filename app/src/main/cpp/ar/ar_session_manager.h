#pragma once
#include "../arcore_c_api.h"
#include <jni.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace arracing {

class ARSessionManager {
public:
    ARSessionManager() = default;
    ~ARSessionManager();

    bool Init(JNIEnv* env, void* context, void* activity);
    void Pause();
    bool Resume();
    bool Update();
    void SetCameraTexture(int tex_id);
    void SetDisplayGeometry(int rotation, int width, int height);

    [[nodiscard]] glm::mat4 ViewMatrix() const;
    [[nodiscard]] glm::mat4 ProjectionMatrix(float near, float far) const;
    [[nodiscard]] bool IsTracking() const;
    [[nodiscard]] bool HasPlanes() const;
    [[nodiscard]] bool IsARAvailable() const { return ar_available_; }

    [[nodiscard]] ArSession* Session() const { return session_; }
    [[nodiscard]] ArFrame*   Frame()   const { return frame_;   }

private:
    ArSession* session_      = nullptr;
    ArFrame*   frame_        = nullptr;
    bool       configured_   = false;
    bool       ar_available_ = false;

    void Configure();
};

} // namespace arracing
