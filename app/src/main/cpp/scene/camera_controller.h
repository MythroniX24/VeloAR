#pragma once
// scene/camera_controller.h
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace arracing {

class CameraController {
public:
    CameraController() = default;

    void Update(const glm::vec3& target_pos,
                float orbit_dx, float orbit_dy,
                float zoom_dist, float dt);

    [[nodiscard]] glm::mat4 ViewMatrix() const;
    [[nodiscard]] glm::vec3 Position()   const { return eye_; }

    float follow_lag = 0.08f;

private:
    glm::vec3 eye_      = {0.0f, 3.0f, 5.0f};
    glm::vec3 target_   = {0.0f, 0.0f, 0.0f};
    float     yaw_      = 0.0f;
    float     pitch_    = 0.35f;   // ← private, underscore convention
    float     cur_dist_ = 5.0f;
};

} // namespace arracing
