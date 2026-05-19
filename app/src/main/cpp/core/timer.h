#pragma once
// core/timer.h — High-resolution frame timer with fixed-step accumulator
#include <chrono>
#include <cstdint>

namespace arracing {

class Timer {
public:
    using Clock     = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration  = std::chrono::duration<double>;

    Timer()
        : start_time_(Clock::now()),
          last_time_(Clock::now()),
          accumulator_(0.0),
          total_time_(0.0),
          frame_count_(0),
          fps_(0.0f),
          fps_timer_(0.0) {}

    // Call at start of each frame
    void Tick() {
        TimePoint now  = Clock::now();
        raw_delta_     = Duration(now - last_time_).count();
        last_time_     = now;

        // Clamp delta to avoid spiral of death after breakpoints/background
        delta_time_    = (raw_delta_ > kMaxDelta) ? kMaxDelta : raw_delta_;

        accumulator_  += delta_time_;
        total_time_   += delta_time_;
        ++frame_count_;

        // FPS calculation every second
        fps_timer_ += delta_time_;
        if (fps_timer_ >= 1.0) {
            fps_       = static_cast<float>(frame_count_ / fps_timer_);
            frame_count_ = 0;
            fps_timer_   = 0.0;
        }
    }

    // Fixed timestep stepping for physics
    // Returns true while accumulator has enough time for one physics step
    bool StepPhysics() {
        if (accumulator_ >= kFixedStep) {
            accumulator_ -= kFixedStep;
            return true;
        }
        return false;
    }

    [[nodiscard]] double  DeltaTime()   const { return delta_time_; }
    [[nodiscard]] float   DeltaF()      const { return static_cast<float>(delta_time_); }
    [[nodiscard]] double  TotalTime()   const { return total_time_; }
    [[nodiscard]] float   FPS()         const { return fps_; }
    [[nodiscard]] double  FixedStep()   const { return kFixedStep; }

    // Interpolation alpha for rendering between physics steps
    [[nodiscard]] float   Alpha()       const {
        return static_cast<float>(accumulator_ / kFixedStep);
    }

private:
    static constexpr double kFixedStep = 1.0 / 120.0; // 120Hz physics
    static constexpr double kMaxDelta  = 0.05;         // 50ms cap

    TimePoint start_time_;
    TimePoint last_time_;
    double    raw_delta_    = 0.0;
    double    delta_time_   = 0.0;
    double    accumulator_  = 0.0;
    double    total_time_   = 0.0;
    uint64_t  frame_count_  = 0;
    float     fps_          = 0.0f;
    double    fps_timer_    = 0.0;
};

} // namespace arracing
