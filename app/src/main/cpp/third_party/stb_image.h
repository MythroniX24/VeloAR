/* stb_image.h — Minimal stub for build compatibility.
 * Replace with real stb_image.h from https://github.com/nothings/stb
 * This stub provides the API surface used by texture_manager.cc.
 *
 * To use the real stb_image:
 *   curl -o app/src/main/cpp/third_party/stb_image.h \
 *        https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
 */
#pragma once
#ifdef STB_IMAGE_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

typedef unsigned char stbi_uc;
static const char* g_stbi_failure_reason = "not loaded";

inline unsigned char* stbi_load_from_memory(
        const unsigned char* buf, int len,
        int* x, int* y, int* channels_in_file, int desired_channels) {
    // Stub: always fails gracefully — real stb_image.h must replace this
    (void)buf; (void)len; (void)desired_channels;
    *x = 0; *y = 0; *channels_in_file = 0;
    g_stbi_failure_reason = "stb_image stub — replace with real stb_image.h";
    return nullptr;
}

inline void stbi_image_free(void* ptr) { free(ptr); }
inline void stbi_set_flip_vertically_on_load(int flag) { (void)flag; }
inline const char* stbi_failure_reason() { return g_stbi_failure_reason; }

#endif // STB_IMAGE_IMPLEMENTATION
