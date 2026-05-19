#pragma once
// core/logger.h — Android logcat wrapper with module tagging
#include <android/log.h>
#include <cstdio>

#define AR_LOG_TAG "ARRacing"

// Info / Warning / Error macros
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  AR_LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  AR_LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, AR_LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, AR_LOG_TAG, __VA_ARGS__)

// OpenGL error check helper
#define CHECK_GL_ERROR(label)                                            \
    do {                                                                 \
        GLenum err = glGetError();                                       \
        if (err != GL_NO_ERROR) {                                        \
            LOGE("GL Error 0x%04x at %s [%s:%d]", err, label,          \
                 __FILE__, __LINE__);                                    \
        }                                                                \
    } while (0)

// ARCore status check helper
#define CHECK_AR_ERROR(status, label)                                    \
    do {                                                                 \
        if ((status) != AR_SUCCESS) {                                    \
            LOGE("ARCore Error %d at %s [%s:%d]", (int)(status),        \
                 label, __FILE__, __LINE__);                             \
        }                                                                \
    } while (0)
