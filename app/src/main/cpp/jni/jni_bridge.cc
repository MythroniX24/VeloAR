// jni/jni_bridge.cc — JNI bridge between Kotlin and C++ engine
// All functions follow Android JNI naming: Java_packageName_ClassName_methodName
#include <jni.h>
#include <android/asset_manager_jni.h>
#include "../scene/scene_manager.h"
#include "../core/logger.h"
#include <memory>
#include <mutex>

// ── Global engine instance ────────────────────────────────────────────────
// Accessed only from GL thread (OnDrawFrame) and main thread (lifecycle).
// Input setters use atomic InputState, so no global lock needed there.
static std::unique_ptr<arracing::SceneManager> g_scene;
static std::mutex g_scene_mutex;

extern "C" {

// ── Lifecycle ─────────────────────────────────────────────────────────────

JNIEXPORT jboolean JNICALL
Java_com_veloar_app_NativeEngine_onCreate(
        JNIEnv* env, jclass /*cls*/,
        jobject context, jobject activity, jobject asset_manager_obj) {

    std::lock_guard<std::mutex> lock(g_scene_mutex);
    g_scene = std::make_unique<arracing::SceneManager>();

    AAssetManager* asset_mgr = AAssetManager_fromJava(env, asset_manager_obj);
    bool ok = g_scene->OnCreate(env,
                                reinterpret_cast<void*>(context),
                                reinterpret_cast<void*>(activity),
                                asset_mgr);
    if (!ok) {
        g_scene.reset();
        LOGE("JNI: onCreate failed");
        return JNI_FALSE;
    }
    LOGI("JNI: onCreate OK");
    return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_veloar_app_NativeEngine_onResume(JNIEnv* /*env*/, jclass /*cls*/) {
    if (g_scene) g_scene->OnResume();
}

JNIEXPORT void JNICALL
Java_com_veloar_app_NativeEngine_onPause(JNIEnv* /*env*/, jclass /*cls*/) {
    if (g_scene) g_scene->OnPause();
}

JNIEXPORT void JNICALL
Java_com_veloar_app_NativeEngine_onDestroy(JNIEnv* /*env*/, jclass /*cls*/) {
    std::lock_guard<std::mutex> lock(g_scene_mutex);
    g_scene.reset();
    LOGI("JNI: onDestroy - engine destroyed");
}

// ── Surface ───────────────────────────────────────────────────────────────

JNIEXPORT void JNICALL
Java_com_veloar_app_NativeEngine_onSurfaceCreated(JNIEnv* /*env*/, jclass /*cls*/) {
    if (g_scene) g_scene->OnSurfaceCreated();
}

JNIEXPORT void JNICALL
Java_com_veloar_app_NativeEngine_onSurfaceChanged(
        JNIEnv* /*env*/, jclass /*cls*/,
        jint rotation, jint width, jint height) {
    if (g_scene) g_scene->OnSurfaceChanged(rotation, width, height);
}

JNIEXPORT void JNICALL
Java_com_veloar_app_NativeEngine_onDrawFrame(JNIEnv* /*env*/, jclass /*cls*/) {
    if (g_scene) g_scene->OnDrawFrame();
}

// ── Input ─────────────────────────────────────────────────────────────────

JNIEXPORT void JNICALL
Java_com_veloar_app_NativeEngine_onJoystick(
        JNIEnv* /*env*/, jclass /*cls*/, jfloat x, jfloat y) {
    if (g_scene) g_scene->OnJoystick(x, y);
}

JNIEXPORT void JNICALL
Java_com_veloar_app_NativeEngine_onThrottle(
        JNIEnv* /*env*/, jclass /*cls*/, jboolean down) {
    if (g_scene) g_scene->OnThrottle(down == JNI_TRUE);
}

JNIEXPORT void JNICALL
Java_com_veloar_app_NativeEngine_onBrake(
        JNIEnv* /*env*/, jclass /*cls*/, jboolean down) {
    if (g_scene) g_scene->OnBrake(down == JNI_TRUE);
}

JNIEXPORT void JNICALL
Java_com_veloar_app_NativeEngine_onHandbrake(
        JNIEnv* /*env*/, jclass /*cls*/, jboolean down) {
    if (g_scene) g_scene->OnHandbrake(down == JNI_TRUE);
}

JNIEXPORT void JNICALL
Java_com_veloar_app_NativeEngine_onCameraDrag(
        JNIEnv* /*env*/, jclass /*cls*/, jfloat dx, jfloat dy) {
    if (g_scene) g_scene->OnCameraDrag(dx, dy);
}

JNIEXPORT void JNICALL
Java_com_veloar_app_NativeEngine_onCameraZoom(
        JNIEnv* /*env*/, jclass /*cls*/, jfloat zoom) {
    if (g_scene) g_scene->OnCameraZoom(zoom);
}

JNIEXPORT void JNICALL
Java_com_veloar_app_NativeEngine_onPlaceCar(JNIEnv* /*env*/, jclass /*cls*/) {
    if (g_scene) g_scene->OnPlaceCar();
}

JNIEXPORT void JNICALL
Java_com_veloar_app_NativeEngine_onResetCar(JNIEnv* /*env*/, jclass /*cls*/) {
    if (g_scene) g_scene->OnResetCar();
}

// ── State queries (called from Kotlin on UI thread) ───────────────────────

JNIEXPORT jboolean JNICALL
Java_com_veloar_app_NativeEngine_isTracking(JNIEnv* /*env*/, jclass /*cls*/) {
    return (g_scene && g_scene->IsTracking()) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_veloar_app_NativeEngine_hasPlanes(JNIEnv* /*env*/, jclass /*cls*/) {
    return (g_scene && g_scene->HasPlanes()) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_veloar_app_NativeEngine_carPlaced(JNIEnv* /*env*/, jclass /*cls*/) {
    return (g_scene && g_scene->CarPlaced()) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jfloat JNICALL
Java_com_veloar_app_NativeEngine_getSpeedKmh(JNIEnv* /*env*/, jclass /*cls*/) {
    return g_scene ? g_scene->SpeedKmh() : 0.0f;
}

JNIEXPORT jfloat JNICALL
Java_com_veloar_app_NativeEngine_getFps(JNIEnv* /*env*/, jclass /*cls*/) {
    return g_scene ? g_scene->FPS() : 0.0f;
}

JNIEXPORT jint JNICALL
Java_com_veloar_app_NativeEngine_getGameState(JNIEnv* /*env*/, jclass /*cls*/) {
    if (!g_scene) return 0;
    return static_cast<jint>(g_scene->State());
}

} // extern "C"
