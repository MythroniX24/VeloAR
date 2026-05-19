#pragma once
// input/input_manager.h
#include "input_state.h"
#include <algorithm>
#include <cmath>

namespace arracing {

class InputManager {
public:
    explicit InputManager(InputState& state) : state_(state) {}

    void SetJoystick(float x, float y) {
        const float len = std::sqrt(x * x + y * y);
        if (len < 0.1f) {
            x = 0.0f; y = 0.0f;
        } else {
            // clamp to unit circle
            const float clamped = len > 1.0f ? 1.0f : len;
            const float nl = clamped / len;
            x *= nl; y *= nl;
        }
        state_.steer_x.store(x);
        state_.steer_y.store(y);
    }

    void SetThrottle(bool d)  { state_.throttle.store(d);  }
    void SetBrake(bool d)     { state_.brake.store(d);     }
    void SetHandbrake(bool d) { state_.handbrake.store(d); }

    void SetCameraDrag(float dx, float dy) {
        state_.cam_delta_x.store(state_.cam_delta_x.load() + dx);
        state_.cam_delta_y.store(state_.cam_delta_y.load() + dy);
    }

    void SetCameraZoom(float z) {
        // manual clamp — avoids std::clamp header issues on some NDK versions
        float cz = z < 2.0f ? 2.0f : (z > 15.0f ? 15.0f : z);
        state_.cam_zoom.store(cz);
    }

    void SetPlaceCar(bool v) { state_.place_car.store(v); }

    void ConsumeCameraDelta() {
        state_.cam_delta_x.store(0.0f);
        state_.cam_delta_y.store(0.0f);
    }

    [[nodiscard]] const InputState& State() const { return state_; }

private:
    InputState& state_;
};

} // namespace arracing
