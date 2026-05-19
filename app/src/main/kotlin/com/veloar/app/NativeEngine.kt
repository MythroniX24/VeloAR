package com.veloar.app

import android.content.res.AssetManager

/**
 * Kotlin object that declares all native methods.
 * The actual implementation lives in ar_racing_native.so (C++).
 * This file is intentionally minimal — all real logic is native.
 */
object NativeEngine {

    init {
        System.loadLibrary("ar_racing_native")
    }

    // ── Lifecycle ─────────────────────────────────────────────────────
    @JvmStatic external fun onCreate(
        context: Any,
        activity: Any,
        assetManager: AssetManager
    ): Boolean

    @JvmStatic external fun onResume()
    @JvmStatic external fun onPause()
    @JvmStatic external fun onDestroy()

    // ── Surface ───────────────────────────────────────────────────────
    @JvmStatic external fun onSurfaceCreated()
    @JvmStatic external fun onSurfaceChanged(rotation: Int, width: Int, height: Int)
    @JvmStatic external fun onDrawFrame()

    // ── Input ─────────────────────────────────────────────────────────
    @JvmStatic external fun onJoystick(x: Float, y: Float)
    @JvmStatic external fun onThrottle(down: Boolean)
    @JvmStatic external fun onBrake(down: Boolean)
    @JvmStatic external fun onHandbrake(down: Boolean)
    @JvmStatic external fun onCameraDrag(dx: Float, dy: Float)
    @JvmStatic external fun onCameraZoom(zoom: Float)
    @JvmStatic external fun onPlaceCar()
    @JvmStatic external fun onResetCar()

    // ── State queries ─────────────────────────────────────────────────
    @JvmStatic external fun isTracking(): Boolean
    @JvmStatic external fun hasPlanes(): Boolean
    @JvmStatic external fun carPlaced(): Boolean
    @JvmStatic external fun getSpeedKmh(): Float
    @JvmStatic external fun getFps(): Float
    @JvmStatic external fun getGameState(): Int  // maps to GameState enum
}
