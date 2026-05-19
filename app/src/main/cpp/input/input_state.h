#pragma once
// input/input_state.h — Plain-data struct shared across systems
#include <atomic>

namespace arracing {

// Thread-safe input state updated from Kotlin touch events via JNI
struct InputState {
    // Joystick: normalized [-1, +1]
    std::atomic<float> steer_x{0.0f};
    std::atomic<float> steer_y{0.0f};

    // Buttons
    std::atomic<bool> throttle{false};
    std::atomic<bool> brake{false};
    std::atomic<bool> handbrake{false};

    // Camera gesture delta (touch drag)
    std::atomic<float> cam_delta_x{0.0f};
    std::atomic<float> cam_delta_y{0.0f};

    // Camera distance pinch
    std::atomic<float> cam_zoom{5.0f};

    // Place car at first detected plane
    std::atomic<bool> place_car{false};
};

} // namespace arracing
