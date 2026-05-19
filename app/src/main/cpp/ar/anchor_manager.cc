// ar/anchor_manager.cc
#include "anchor_manager.h"
#include "../core/logger.h"
#include <glm/gtc/type_ptr.hpp>

namespace arracing {

bool AnchorManager::AttachToPlane(ArSession* session, ArFrame* frame,
                                   const ArPlane* plane) {
    // Release old anchor first
    if (anchor_) {
        ArAnchor_release(anchor_);
        anchor_ = nullptr;
    }

    // Get center pose of the plane
    ArPose* pose = nullptr;
    ArPose_create(session, nullptr, &pose);
    ArPlane_getCenterPose(session, plane, pose);

    // Create anchor at plane center
    ArStatus status = ArSession_acquireNewAnchor(session, pose, &anchor_);
    ArPose_destroy(pose);

    if (status != AR_SUCCESS || !anchor_) {
        LOGE("AnchorManager: Failed to create anchor (%d)", (int)status);
        anchor_ = nullptr;
        return false;
    }

    LOGI("AnchorManager: Anchor created on plane");
    return true;
}

void AnchorManager::Release(ArSession* /*session*/) {
    if (anchor_) {
        ArAnchor_release(anchor_);
        anchor_ = nullptr;
        LOGI("AnchorManager: Anchor released");
    }
}

bool AnchorManager::IsTracking(ArSession* session) const {
    if (!anchor_) return false;
    ArTrackingState state;
    ArAnchor_getTrackingState(session, anchor_, &state);
    return state == AR_TRACKING_STATE_TRACKING;
}

std::optional<glm::mat4> AnchorManager::GetAnchorMatrix(ArSession* session) const {
    if (!anchor_) return std::nullopt;

    ArTrackingState state;
    ArAnchor_getTrackingState(session, anchor_, &state);
    if (state != AR_TRACKING_STATE_TRACKING) return std::nullopt;

    // Get anchor pose
    ArPose* pose = nullptr;
    ArPose_create(session, nullptr, &pose);
    ArAnchor_getPose(session, anchor_, pose);

    // Raw: [qx, qy, qz, qw, tx, ty, tz]
    float raw[7];
    ArPose_getPoseRaw(session, pose, raw);
    ArPose_destroy(pose);

    // Build model matrix
    glm::quat q(raw[3], raw[0], raw[1], raw[2]);
    glm::vec3 t(raw[4], raw[5], raw[6]);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), t) * glm::mat4_cast(q);

    return model;
}

AnchorManager::~AnchorManager() {
    if (anchor_) {
        ArAnchor_release(anchor_);
        anchor_ = nullptr;
    }
}

} // namespace arracing
