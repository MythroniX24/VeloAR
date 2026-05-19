#pragma once
// renderer/texture_manager.h — Texture loading and caching
#include <GLES3/gl3.h>
#include <string>
#include <unordered_map>
#include "../core/asset_manager.h"

namespace arracing {

class TextureManager {
public:
    explicit TextureManager(AssetManager& assets) : assets_(assets) {}
    ~TextureManager();

    // Load PNG from assets, upload to GL, cache by path
    [[nodiscard]] GLuint LoadTexture(const std::string& asset_path,
                                     bool gen_mipmaps = true);

    // Create a 1x1 solid colour texture
    [[nodiscard]] GLuint CreateSolidColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a,
                                          const std::string& name);

    [[nodiscard]] GLuint Get(const std::string& name) const;
    void Cleanup();

private:
    AssetManager& assets_;
    std::unordered_map<std::string, GLuint> cache_;
};

} // namespace arracing
