// renderer/texture_manager.cc — PNG/JPEG texture loading with stb_image
// stb_image is a single-header library - we define implementation here only once
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"
#include "texture_manager.h"
#include "../core/logger.h"

namespace arracing {

GLuint TextureManager::LoadTexture(const std::string& asset_path,
                                   bool gen_mipmaps) {
    // Return cached version if already loaded
    auto it = cache_.find(asset_path);
    if (it != cache_.end()) return it->second;

    // Load raw bytes from APK assets
    auto file_bytes = assets_.LoadFile(asset_path);
    if (file_bytes.empty()) {
        LOGE("TextureManager: Empty asset '%s'", asset_path.c_str());
        return 0;
    }

    // Decode PNG/JPEG via stb_image
    int w, h, channels;
    stbi_set_flip_vertically_on_load(0); // Keep OpenGL convention
    unsigned char* pixels = stbi_load_from_memory(
        file_bytes.data(), static_cast<int>(file_bytes.size()),
        &w, &h, &channels, 0);

    if (!pixels) {
        LOGE("TextureManager: stb_image decode failed for '%s': %s",
             asset_path.c_str(), stbi_failure_reason());
        return 0;
    }

    // Choose GL internal format based on channel count
    GLenum fmt = GL_RGBA;
    if      (channels == 1) fmt = GL_RED;
    else if (channels == 2) fmt = GL_RG;
    else if (channels == 3) fmt = GL_RGB;

    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    // Upload pixels
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, pixels);

    if (gen_mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(pixels);
    CHECK_GL_ERROR("TextureManager::LoadTexture");

    LOGI("TextureManager: Loaded '%s' (%dx%d ch=%d) -> texid=%u",
         asset_path.c_str(), w, h, channels, tex_id);

    cache_[asset_path] = tex_id;
    return tex_id;
}

GLuint TextureManager::CreateSolidColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a,
                                        const std::string& name) {
    auto it = cache_.find(name);
    if (it != cache_.end()) return it->second;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    uint8_t pixel[4] = {r, g, b, a};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    cache_[name] = tex;
    return tex;
}

GLuint TextureManager::Get(const std::string& name) const {
    auto it = cache_.find(name);
    return (it != cache_.end()) ? it->second : 0;
}

void TextureManager::Cleanup() {
    for (auto& [name, id] : cache_) {
        glDeleteTextures(1, &id);
    }
    cache_.clear();
}

TextureManager::~TextureManager() { Cleanup(); }

} // namespace arracing
