// core/asset_manager.cc — AAssetManager implementation
#include "asset_manager.h"
#include "logger.h"
#include <android/asset_manager.h>

namespace arracing {

std::vector<uint8_t> AssetManager::LoadFile(const std::string& path) const {
    // Open asset from APK
    AAsset* asset = AAssetManager_open(mgr_, path.c_str(), AASSET_MODE_BUFFER);
    if (!asset) {
        LOGE("AssetManager: Failed to open '%s'", path.c_str());
        return {};
    }

    // Get size and copy into vector
    const off_t size = AAsset_getLength(asset);
    std::vector<uint8_t> buf(static_cast<size_t>(size));
    AAsset_read(asset, buf.data(), static_cast<size_t>(size));
    AAsset_close(asset);

    LOGD("AssetManager: Loaded '%s' (%d bytes)", path.c_str(), (int)size);
    return buf;
}

std::string AssetManager::LoadString(const std::string& path) const {
    auto bytes = LoadFile(path);
    return std::string(reinterpret_cast<char*>(bytes.data()), bytes.size());
}

} // namespace arracing
