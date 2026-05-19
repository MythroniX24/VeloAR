package com.veloar.app

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View
import kotlin.math.min
import kotlin.math.sqrt

/**
 * Virtual joystick View.
 * Reports normalized (x, y) in [-1, +1] via onMove callback.
 * Renders base circle + movable knob.
 */
class VirtualJoystick @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null
) : View(context, attrs) {

    var onMove: ((x: Float, y: Float) -> Unit)? = null

    private val paintBase = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.argb(80, 255, 255, 255)
        style = Paint.Style.FILL
    }
    private val paintBorder = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.argb(160, 255, 255, 255)
        style = Paint.Style.STROKE
        strokeWidth = 4f
    }
    private val paintKnob = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.argb(200, 100, 200, 255)
        style = Paint.Style.FILL
    }

    private var centerX = 0f
    private var centerY = 0f
    private var baseRadius = 0f
    private var knobRadius = 0f
    private var knobX = 0f
    private var knobY = 0f
    private var activePointerId = -1

    override fun onSizeChanged(w: Int, h: Int, oldw: Int, oldh: Int) {
        centerX    = w / 2f
        centerY    = h / 2f
        baseRadius = min(w, h) / 2f * 0.9f
        knobRadius = baseRadius * 0.38f
        knobX      = centerX
        knobY      = centerY
    }

    override fun onDraw(canvas: Canvas) {
        // Base ring
        canvas.drawCircle(centerX, centerY, baseRadius, paintBase)
        canvas.drawCircle(centerX, centerY, baseRadius, paintBorder)
        // Knob
        canvas.drawCircle(knobX, knobY, knobRadius, paintKnob)
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                activePointerId = event.getPointerId(0)
                updateKnob(event.x, event.y)
            }
            MotionEvent.ACTION_MOVE -> {
                val idx = event.findPointerIndex(activePointerId)
                if (idx >= 0) updateKnob(event.getX(idx), event.getY(idx))
            }
            MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                activePointerId = -1
                resetKnob()
            }
        }
        return true
    }

    private fun updateKnob(touchX: Float, touchY: Float) {
        val dx = touchX - centerX
        val dy = touchY - centerY
        val dist = sqrt(dx * dx + dy * dy)
        val clampedDist = dist.coerceAtMost(baseRadius)
        val ratio = if (dist > 0) clampedDist / dist else 0f

        knobX = centerX + dx * ratio
        knobY = centerY + dy * ratio
        invalidate()

        // Normalized output in [-1, +1]
        val nx = (knobX - centerX) / baseRadius
        val ny = (knobY - centerY) / baseRadius
        onMove?.invoke(nx, ny)
    }

    private fun resetKnob() {
        knobX = centerX
        knobY = centerY
        invalidate()
        onMove?.invoke(0f, 0f)
    }
}
