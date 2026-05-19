// scene/camera_controller.cc
#include "camera_controller.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace arracing {

void CameraController::Update(const glm::vec3& target_pos,
                               float orbit_dx, float orbit_dy,
                               float zoom_dist, float dt) {
    yaw_      += orbit_dx * 1.2f * dt;
    pitch_     = std::clamp(pitch_ - orbit_dy * 0.8f * dt, 0.05f, 1.4f);
    cur_dist_ += (zoom_dist - cur_dist_) * (1.0f - std::pow(0.01f, dt));

    float horiz = cur_dist_ * std::cos(pitch_);
    float vert  = cur_dist_ * std::sin(pitch_);

    glm::vec3 desired_eye(
        target_pos.x + horiz * std::sin(yaw_),
        target_pos.y + vert,
        target_pos.z + horiz * std::cos(yaw_)
    );

    float alpha = 1.0f - std::pow(follow_lag, dt);
    eye_    = glm::mix(eye_,    desired_eye,  alpha);
    target_ = glm::mix(target_, target_pos,   alpha * 2.0f);
}

glm::mat4 CameraController::ViewMatrix() const {
    return glm::lookAt(eye_, target_, glm::vec3(0.0f, 1.0f, 0.0f));
}

} // namespace arracing
