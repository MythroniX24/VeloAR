// scene/scene_manager.cc — Main AR Racing game loop with ARCore fallback
#include "scene_manager.h"
#include "../core/logger.h"
#include <GLES3/gl3.h>
#include <glm/gtc/matrix_transform.hpp>

namespace arracing {

bool SceneManager::OnCreate(JNIEnv* env, void* context, void* activity,
                             AAssetManager* asset_mgr) {
    LOGI("SceneManager: OnCreate");

    asset_mgr_ = std::make_unique<AssetManager>(asset_mgr);
    input_mgr_ = std::make_unique<InputManager>(input_state_);

    ar_session_    = std::make_unique<ARSessionManager>();
    plane_tracker_ = std::make_unique<PlaneTracker>();
    anchor_mgr_    = std::make_unique<AnchorManager>();

    // Init ARCore - won't crash even if unavailable
    bool ar_ok = ar_session_->Init(env, context, activity);
    if (!ar_ok) {
        LOGE("SceneManager: ARCore init returned false - continuing anyway");
    }
    ar_ready_ = true;

    physics_world_ = std::make_unique<PhysicsWorld>();
    physics_world_->Init();

    LOGI("SceneManager: OnCreate complete (ARCore=%s)",
         ar_session_->IsARAvailable() ? "YES" : "NO (fallback)");
    return true;
}

void SceneManager::OnResume() {
    if (ar_session_ && ar_session_->IsARAvailable()) ar_session_->Resume();
}

void SceneManager::OnPause() {
    if (ar_session_ && ar_session_->IsARAvailable()) ar_session_->Pause();
}

void SceneManager::OnSurfaceCreated() {
    LOGI("SceneManager: OnSurfaceCreated");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    bg_renderer_ = std::make_unique<BackgroundRenderer>();
    bg_renderer_->Init(*asset_mgr_);

    // Only set camera texture if ARCore available
    if (ar_session_ && ar_session_->IsARAvailable() && bg_renderer_) {
        ar_session_->SetCameraTexture(
            static_cast<int>(bg_renderer_->CameraTextureId()));
    }

    plane_renderer_ = std::make_unique<PlaneRenderer>();
    plane_renderer_->Init(*asset_mgr_);

    mesh_renderer_ = std::make_unique<MeshRenderer>();
    shadow_map_    = std::make_unique<ShadowMap>();
    shadow_map_->Init();

    texture_mgr_ = std::make_unique<TextureManager>(*asset_mgr_);
    camera_      = std::make_unique<CameraController>();

    car_ = std::make_unique<CarEntity>();
    car_->Init(*physics_world_, glm::vec3(0.0f), *asset_mgr_, *mesh_renderer_);

    physics_world_->CreateGroundPlane(0.0f);

    gl_ready_ = true;

    // If no ARCore, auto-place car at origin after GL ready
    if (!ar_session_->IsARAvailable()) {
        state_      = GameState::kDriving;
        anchor_mat_ = glm::mat4(1.0f);
        LOGI("SceneManager: No ARCore - auto-placed car at origin");
    } else {
        state_ = GameState::kSearchingPlane;
    }

    LOGI("SceneManager: GL ready");
}

void SceneManager::OnSurfaceChanged(int rotation, int w, int h) {
    screen_w_    = w;
    screen_h_    = h;
    display_rot_ = rotation;
    glViewport(0, 0, w, h);
    if (ar_session_ && ar_session_->IsARAvailable())
        ar_session_->SetDisplayGeometry(rotation, w, h);
}

void SceneManager::OnDrawFrame() {
    if (!gl_ready_ || !ar_ready_) return;

    timer_.Tick();

    // ARCore update only if available
    if (ar_session_->IsARAvailable()) {
        if (!ar_session_->Update()) return;

        bool tracking = ar_session_->IsTracking();
        if (state_ == GameState::kSearchingPlane && tracking) {
            if (plane_tracker_->GetBestFloorPlane(ar_session_->Session()))
                state_ = GameState::kPlacingCar;
        }

        if (state_ == GameState::kPlacingCar && input_state_.place_car.load()) {
            TryPlaceCar();
            input_state_.place_car.store(false);
        }

        if (anchor_mgr_->HasAnchor()) {
            auto mat = anchor_mgr_->GetAnchorMatrix(ar_session_->Session());
            if (mat.has_value()) anchor_mat_ = mat.value();
        }
    } else {
        // No ARCore - always in driving state
        state_ = GameState::kDriving;
        if (input_state_.place_car.load()) {
            input_state_.place_car.store(false);
        }
    }

    // Physics
    if (state_ == GameState::kDriving) {
        while (timer_.StepPhysics()) {
            car_->UpdatePhysics(input_state_,
                                static_cast<float>(timer_.FixedStep()));
        }
        physics_world_->Step(timer_.DeltaF());
    }

    // Camera
    glm::vec3 car_pos{0.0f};
    if (state_ == GameState::kDriving) {
        glm::vec3 phys_pos = car_->State().position;
        car_pos = glm::vec3(anchor_mat_ * glm::vec4(phys_pos, 1.0f));
    }

    float cam_dx = input_state_.cam_delta_x.load();
    float cam_dy = input_state_.cam_delta_y.load();
    float cam_z  = input_state_.cam_zoom.load();
    input_mgr_->ConsumeCameraDelta();
    camera_->Update(car_pos, cam_dx, cam_dy, cam_z, timer_.DeltaF());

    RenderFrame();
}

void SceneManager::RenderFrame() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 proj, view, vp;

    if (ar_session_->IsARAvailable() && ar_session_->Session()) {
        auto* session = ar_session_->Session();
        auto* frame   = ar_session_->Frame();

        // Draw camera background
        if (bg_renderer_) bg_renderer_->Draw(session, frame);

        proj = ar_session_->ProjectionMatrix(0.05f, 100.0f);
        view = ar_session_->ViewMatrix();

        // Draw planes when searching
        if ((state_ == GameState::kSearchingPlane ||
             state_ == GameState::kPlacingCar) && plane_renderer_) {
            auto planes = plane_tracker_->GetTrackedPlanes(session);
            for (auto& pi : planes)
                plane_renderer_->DrawPlane(proj, view, session, pi.plane);
        }
    } else {
        // Fallback: simple perspective + orbit camera
        float aspect = (screen_h_ > 0)
            ? static_cast<float>(screen_w_) / screen_h_ : 16.0f/9.0f;
        proj = glm::perspective(glm::radians(60.0f), aspect, 0.05f, 100.0f);
        view = camera_->ViewMatrix();
    }

    vp = proj * view;

    // Render car
    if (state_ == GameState::kDriving && car_ && mesh_renderer_) {
        glm::vec3 scene_center = glm::vec3(anchor_mat_[3]);
        shadow_map_->BeginShadowPass();
        shadow_map_->EndShadowPass(screen_w_, screen_h_);
        car_->Render(*mesh_renderer_, vp, light_dir_,
                     camera_->Position(), anchor_mat_);
    }
}

void SceneManager::TryPlaceCar() {
    auto* session = ar_session_->Session();
    if (!session) return;
    const ArPlane* plane = plane_tracker_->GetBestFloorPlane(session);
    if (!plane) return;

    bool ok = anchor_mgr_->AttachToPlane(session, ar_session_->Frame(), plane);
    if (!ok) return;

    auto mat = anchor_mgr_->GetAnchorMatrix(session);
    if (!mat.has_value()) return;

    anchor_mat_ = mat.value();
    glm::vec3 pos = glm::vec3(anchor_mat_[3]);

    car_->ResetToAnchor(anchor_mat_);
    physics_world_->CreateGroundPlane(pos.y);
    state_ = GameState::kDriving;
    LOGI("SceneManager: Car placed at (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
}

void SceneManager::OnJoystick(float x, float y)     { input_mgr_->SetJoystick(x, y);     }
void SceneManager::OnThrottle(bool d)               { input_mgr_->SetThrottle(d);         }
void SceneManager::OnBrake(bool d)                  { input_mgr_->SetBrake(d);            }
void SceneManager::OnHandbrake(bool d)              { input_mgr_->SetHandbrake(d);        }
void SceneManager::OnCameraDrag(float dx, float dy) { input_mgr_->SetCameraDrag(dx, dy);  }
void SceneManager::OnCameraZoom(float z)            { input_mgr_->SetCameraZoom(z);       }
void SceneManager::OnPlaceCar()                     { input_mgr_->SetPlaceCar(true);      }
void SceneManager::OnResetCar() {
    if (state_ == GameState::kDriving) car_->ResetToAnchor(anchor_mat_);
}

bool  SceneManager::IsTracking() const {
    if (!ar_session_->IsARAvailable()) return true;
    return ar_session_->IsTracking();
}
bool  SceneManager::HasPlanes()  const {
    if (!ar_session_->IsARAvailable()) return true;
    return ar_session_->HasPlanes();
}
bool  SceneManager::CarPlaced()  const { return state_ == GameState::kDriving; }
float SceneManager::SpeedKmh()   const { return car_ ? car_->State().speed_kmh : 0.0f; }
float SceneManager::FPS()        const { return timer_.FPS(); }

} // namespace arracing
