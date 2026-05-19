#pragma once
// scene/scene_manager.h — Orchestrates the full game loop
#include "../ar/ar_session_manager.h"
#include "../ar/plane_tracker.h"
#include "../ar/anchor_manager.h"
#include "../physics/physics_world.h"
#include "../car/car_entity.h"
#include "../renderer/background_renderer.h"
#include "../renderer/plane_renderer.h"
#include "../renderer/mesh_renderer.h"
#include "../renderer/shadow_map.h"
#include "../renderer/texture_manager.h"
#include "../scene/camera_controller.h"
#include "../input/input_manager.h"
#include "../input/input_state.h"
#include "../core/timer.h"
#include "../core/asset_manager.h"
#include <jni.h>
#include <memory>

namespace arracing {

// Game state machine
enum class GameState {
    kInitializing,   // AR session not yet ready
    kSearchingPlane, // Scanning floor for planes
    kPlacingCar,     // First plane found, waiting for user to place car
    kDriving,        // Car placed, physics running
};

class SceneManager {
public:
    SceneManager() = default;
    ~SceneManager() = default;

    // ── Android lifecycle ──────────────────────────────────────────────
    bool OnCreate(JNIEnv* env, void* context, void* activity,
                  AAssetManager* asset_mgr);
    void OnResume();
    void OnPause();
    void OnSurfaceCreated();
    void OnSurfaceChanged(int rotation, int width, int height);

    // ── Frame loop (called by GLSurfaceView each frame) ────────────────
    void OnDrawFrame();

    // ── Input from JNI ─────────────────────────────────────────────────
    void OnJoystick(float x, float y);
    void OnThrottle(bool down);
    void OnBrake(bool down);
    void OnHandbrake(bool down);
    void OnCameraDrag(float dx, float dy);
    void OnCameraZoom(float zoom);
    void OnPlaceCar();
    void OnResetCar();

    // ── Query state for Kotlin UI ──────────────────────────────────────
    [[nodiscard]] bool      IsTracking()   const;
    [[nodiscard]] bool      HasPlanes()    const;
    [[nodiscard]] bool      CarPlaced()    const;
    [[nodiscard]] float     SpeedKmh()     const;
    [[nodiscard]] float     FPS()          const;
    [[nodiscard]] GameState State()        const { return state_; }

private:
    void RenderFrame();
    void UpdatePhysics();
    void TryPlaceCar();

    // Systems (owned)
    std::unique_ptr<AssetManager>     asset_mgr_;
    std::unique_ptr<ARSessionManager> ar_session_;
    std::unique_ptr<PlaneTracker>     plane_tracker_;
    std::unique_ptr<AnchorManager>    anchor_mgr_;
    std::unique_ptr<PhysicsWorld>     physics_world_;
    std::unique_ptr<CarEntity>        car_;
    std::unique_ptr<BackgroundRenderer> bg_renderer_;
    std::unique_ptr<PlaneRenderer>    plane_renderer_;
    std::unique_ptr<MeshRenderer>     mesh_renderer_;
    std::unique_ptr<ShadowMap>        shadow_map_;
    std::unique_ptr<TextureManager>   texture_mgr_;
    std::unique_ptr<CameraController> camera_;
    std::unique_ptr<InputManager>     input_mgr_;

    InputState input_state_;
    Timer      timer_;

    GameState  state_       = GameState::kInitializing;
    int        screen_w_    = 1;
    int        screen_h_    = 1;
    int        display_rot_ = 0;
    bool       gl_ready_    = false;
    bool       ar_ready_    = false;

    // AR anchor model matrix - updated each frame from ARCore
    glm::mat4  anchor_mat_  = glm::mat4(1.0f);

    // Lighting
    glm::vec3  light_dir_   = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f));
};

} // namespace arracing
