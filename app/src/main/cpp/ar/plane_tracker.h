#pragma once
// ar/plane_tracker.h — Queries ARCore for detected planes
#include "../arcore_c_api.h"
#include <vector>

namespace arracing {

struct PlaneInfo {
    const ArPlane* plane;
    float          area;       // estimated plane area m²
};

class PlaneTracker {
public:
    PlaneTracker() = default;

    // Returns list of currently tracked planes sorted by area (largest first)
    [[nodiscard]] std::vector<PlaneInfo>
    GetTrackedPlanes(ArSession* session) const;

    // Returns largest horizontal plane or nullptr
    [[nodiscard]] const ArPlane*
    GetBestFloorPlane(ArSession* session) const;
};

} // namespace arracing
