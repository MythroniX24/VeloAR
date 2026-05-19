#pragma once
// core/asset_manager.h — Wrapper around Android AAssetManager
#include <android/asset_manager.h>
#include <string>
#include <vector>

namespace arracing {

class AssetManager {
public:
    explicit AssetManager(AAssetManager* mgr) : mgr_(mgr) {}

    // Load entire file into byte vector. Returns empty on failure.
    [[nodiscard]] std::vector<uint8_t> LoadFile(const std::string& path) const;

    // Load file as string (shader source, etc.)
    [[nodiscard]] std::string LoadString(const std::string& path) const;

    [[nodiscard]] AAssetManager* Raw() const { return mgr_; }

private:
    AAssetManager* mgr_;
};

} // namespace arracing
