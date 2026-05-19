// ar/plane_tracker.cc
#include "plane_tracker.h"
#include "../core/logger.h"
#include <algorithm>
#include <cmath>

namespace arracing {

std::vector<PlaneInfo>
PlaneTracker::GetTrackedPlanes(ArSession* session) const {
    std::vector<PlaneInfo> result;

    ArTrackableList* plane_list = nullptr;
    ArTrackableList_create(session, &plane_list);
    ArSession_getAllTrackables(session, AR_TRACKABLE_PLANE, plane_list);

    int32_t count = 0;
    ArTrackableList_getSize(session, plane_list, &count);

    for (int32_t i = 0; i < count; ++i) {
        ArTrackable* trackable = nullptr;
        ArTrackableList_acquireItem(session, plane_list, i, &trackable);
        ArPlane* plane = ArAsPlane(trackable);

        // Skip subsumed planes (child planes consumed by parent)
        ArPlane* parent = nullptr;
        ArPlane_acquireSubsumedBy(session, plane, &parent);
        if (parent) {
            ArTrackable_release(ArAsTrackable(parent));
            ArTrackable_release(trackable);
            continue;
        }

        // Only tracking planes
        ArTrackingState state;
        ArTrackable_getTrackingState(session, trackable, &state);
        if (state != AR_TRACKING_STATE_TRACKING) {
            ArTrackable_release(trackable);
            continue;
        }

        // Only horizontal (floor) planes
        ArPlaneType type;
        ArPlane_getType(session, plane, &type);
        if (type != AR_PLANE_HORIZONTAL_UPWARD_FACING) {
            ArTrackable_release(trackable);
            continue;
        }

        // Estimate area from polygon
        float extent_x = 0.0f, extent_z = 0.0f;
        ArPlane_getExtentX(session, plane, &extent_x);
        ArPlane_getExtentZ(session, plane, &extent_z);
        float area = extent_x * extent_z;

        result.push_back({plane, area});
        // Note: do NOT release trackable here; caller must use plane before
        // next frame. In our case we use it immediately.
        ArTrackable_release(trackable);
    }

    ArTrackableList_destroy(plane_list);

    // Sort by area descending
    std::sort(result.begin(), result.end(),
              [](const PlaneInfo& a, const PlaneInfo& b) {
                  return a.area > b.area;
              });

    return result;
}

const ArPlane* PlaneTracker::GetBestFloorPlane(ArSession* session) const {
    auto planes = GetTrackedPlanes(session);
    if (planes.empty()) return nullptr;
    return planes[0].plane;
}

} // namespace arracing
