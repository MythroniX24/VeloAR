#pragma once
// ar/anchor_manager.h — Manages world-space AR anchors
#include "../arcore_c_api.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <optional>

namespace arracing {

class AnchorManager {
public:
    AnchorManager() = default;
    ~AnchorManager();

    // Attach anchor to first tracked horizontal plane
    bool AttachToPlane(ArSession* session, ArFrame* frame,
                       const ArPlane* plane);

    // Release anchor (e.g., when relocating car)
    void Release(ArSession* session);

    // Compute world-space model matrix from current anchor pose
    [[nodiscard]] std::optional<glm::mat4> GetAnchorMatrix(ArSession* session) const;

    [[nodiscard]] bool HasAnchor() const { return anchor_ != nullptr; }
    [[nodiscard]] bool IsTracking(ArSession* session) const;

private:
    ArAnchor* anchor_ = nullptr;
};

} // namespace arracing
