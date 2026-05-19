// ar/ar_session_manager.cc — ARCore session with graceful fallback
#include "ar_session_manager.h"
#include "../core/logger.h"
#include <glm/gtc/type_ptr.hpp>

namespace arracing {

bool ARSessionManager::Init(JNIEnv* env, void* context, void* activity) {
    // Check availability first - don't crash if ARCore missing
    ArAvailability avail = AR_AVAILABILITY_UNKNOWN_CHECKING;
    ArCoreApk_checkAvailability(env, context, &avail);

    LOGI("ARSessionManager: Availability = %d", (int)avail);

    if (avail == AR_AVAILABILITY_UNSUPPORTED_DEVICE_NOT_CAPABLE) {
        LOGW("ARSessionManager: Device does not support ARCore - running in fallback mode");
        ar_available_ = false;
        return true; // Don't crash - fallback mode
    }

    // Try to request install
    ArInstallStatus install_status = AR_INSTALL_STATUS_INSTALLED;
    ArStatus req = ArCoreApk_requestInstall(env, activity, false, &install_status);
    if (req != AR_SUCCESS || install_status == AR_INSTALL_STATUS_INSTALL_REQUESTED) {
        LOGW("ARSessionManager: ARCore not installed - fallback mode");
        ar_available_ = false;
        return true;
    }

    // Try to create session
    ArStatus status = ArSession_create(env, context, &session_);
    if (status != AR_SUCCESS || !session_) {
        LOGW("ARSessionManager: Session create failed (%d) - fallback mode", (int)status);
        ar_available_ = false;
        session_ = nullptr;
        return true;
    }

    ArFrame_create(session_, &frame_);
    Configure();

    ar_available_ = true;
    LOGI("ARSessionManager: ARCore initialized successfully");
    return true;
}

void ARSessionManager::Configure() {
    if (!session_) return;

    ArConfig* config = nullptr;
    ArConfig_create(session_, &config);
    ArConfig_setPlaneFindingMode(session_, config, AR_PLANE_FINDING_MODE_HORIZONTAL);
    ArConfig_setFocusMode(session_, config, AR_FOCUS_MODE_AUTO);
    ArConfig_setUpdateMode(session_, config, AR_UPDATE_MODE_LATEST_CAMERA_IMAGE);
    ArSession_configure(session_, config);
    ArConfig_destroy(config);
    configured_ = true;
}

void ARSessionManager::SetCameraTexture(int tex_id) {
    if (session_) ArSession_setCameraTextureName(session_, tex_id);
}

void ARSessionManager::SetDisplayGeometry(int rotation, int width, int height) {
    if (session_) ArSession_setDisplayGeometry(session_, rotation, width, height);
}

void ARSessionManager::Pause() {
    if (session_) ArSession_pause(session_);
}

bool ARSessionManager::Resume() {
    if (!session_) return true; // fallback mode ok
    ArStatus status = ArSession_resume(session_);
    return status == AR_SUCCESS;
}

bool ARSessionManager::Update() {
    if (!session_) return true; // fallback mode
    ArStatus status = ArSession_update(session_, frame_);
    return status == AR_SUCCESS;
}

bool ARSessionManager::IsTracking() const {
    if (!session_ || !frame_) return false;
    ArCamera* camera = nullptr;
    ArFrame_acquireCamera(session_, frame_, &camera);
    if (!camera) return false;
    ArTrackingState state;
    ArCamera_getTrackingState(session_, camera, &state);
    ArCamera_release(camera);
    return state == AR_TRACKING_STATE_TRACKING;
}

bool ARSessionManager::HasPlanes() const {
    if (!session_) return false;
    ArTrackableList* list = nullptr;
    ArTrackableList_create(session_, &list);
    ArSession_getAllTrackables(session_, AR_TRACKABLE_PLANE, list);
    int32_t count = 0;
    ArTrackableList_getSize(session_, list, &count);
    ArTrackableList_destroy(list);
    return count > 0;
}

glm::mat4 ARSessionManager::ViewMatrix() const {
    if (!session_ || !frame_) return glm::mat4(1.0f);
    ArCamera* camera = nullptr;
    ArFrame_acquireCamera(session_, frame_, &camera);
    float view[16];
    ArCamera_getViewMatrix(session_, camera, view);
    ArCamera_release(camera);
    return glm::make_mat4(view);
}

glm::mat4 ARSessionManager::ProjectionMatrix(float near, float far) const {
    if (!session_ || !frame_) {
        // Fallback projection matrix
        return glm::perspective(glm::radians(60.0f), 16.0f/9.0f, near, far);
    }
    ArCamera* camera = nullptr;
    ArFrame_acquireCamera(session_, frame_, &camera);
    float proj[16];
    ArCamera_getProjectionMatrix(session_, camera, near, far, proj);
    ArCamera_release(camera);
    return glm::make_mat4(proj);
}

ARSessionManager::~ARSessionManager() {
    if (frame_)   { ArFrame_destroy(frame_);     frame_   = nullptr; }
    if (session_) { ArSession_destroy(session_); session_ = nullptr; }
}

} // namespace arracing
