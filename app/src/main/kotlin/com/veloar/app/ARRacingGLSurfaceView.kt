package com.veloar.app

import android.content.Context
import android.opengl.GLSurfaceView
import android.view.MotionEvent
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import kotlin.math.sqrt

/**
 * GLSurfaceView that:
 * 1. Configures OpenGL ES 3.0 context
 * 2. Routes surface lifecycle to C++
 * 3. Routes multi-touch camera gestures to C++
 * All rendering logic lives in native code.
 */
class ARRacingGLSurfaceView(context: Context) : GLSurfaceView(context) {

    private val renderer = NativeRenderer()

    // Track pinch-to-zoom
    private var lastPinchDist = 0f
    private var lastTouchX = 0f
    private var lastTouchY = 0f
    private var currentZoom = 5f

    init {
        setEGLContextClientVersion(3)
        setEGLConfigChooser(8, 8, 8, 8, 16, 0)
        setRenderer(renderer)
        renderMode = RENDERMODE_CONTINUOUSLY
        preserveEGLContextOnPause = true
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                lastTouchX = event.x
                lastTouchY = event.y
            }
            MotionEvent.ACTION_MOVE -> {
                if (event.pointerCount == 1) {
                    // Single finger: orbit camera
                    val dx = (event.x - lastTouchX) * 0.003f
                    val dy = (event.y - lastTouchY) * 0.003f
                    queueEvent { NativeEngine.onCameraDrag(dx, dy) }
                    lastTouchX = event.x
                    lastTouchY = event.y
                } else if (event.pointerCount == 2) {
                    // Two fingers: pinch-to-zoom
                    val dx = event.getX(0) - event.getX(1)
                    val dy = event.getY(0) - event.getY(1)
                    val dist = sqrt(dx * dx + dy * dy)
                    if (lastPinchDist > 0f) {
                        val delta = (lastPinchDist - dist) * 0.01f
                        currentZoom = (currentZoom + delta).coerceIn(2f, 15f)
                        queueEvent { NativeEngine.onCameraZoom(currentZoom) }
                    }
                    lastPinchDist = dist
                }
            }
            MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                lastPinchDist = 0f
            }
        }
        return true
    }

    // Called by MainActivity on display rotation change
    fun updateDisplayRotation(rotation: Int) {
        queueEvent { NativeEngine.onSurfaceChanged(rotation, width, height) }
    }

    inner class NativeRenderer : Renderer {
        private var displayRotation = 0

        fun setDisplayRotation(r: Int) { displayRotation = r }

        override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
            NativeEngine.onSurfaceCreated()
        }

        override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
            NativeEngine.onSurfaceChanged(displayRotation, width, height)
        }

        override fun onDrawFrame(gl: GL10?) {
            NativeEngine.onDrawFrame()
        }
    }
}
