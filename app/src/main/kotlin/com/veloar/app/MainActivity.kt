package com.veloar.app

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.view.View
import android.view.WindowManager
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch

class MainActivity : AppCompatActivity() {

    private lateinit var glView: ARRacingGLSurfaceView
    private lateinit var tvStatus: TextView
    private lateinit var tvSpeed: TextView
    private lateinit var tvFps: TextView
    private lateinit var tvHint: TextView
    private lateinit var btnPlaceCar: View
    private lateinit var btnThrottle: View
    private lateinit var btnBrake: View
    private lateinit var btnHandbrake: View
    private lateinit var btnReset: View
    private lateinit var joystick: VirtualJoystick

    @Volatile private var engineReady = false

    companion object {
        private const val CAM_PERM = 1001
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        setImmersiveMode()

        setContentView(R.layout.activity_main)
        bindViews()
        setupButtons()
        setupJoystick()

        if (hasCameraPermission()) initEngineWhenReady()
        else requestCameraPermission()

        startHudLoop()
    }

    private fun bindViews() {
        glView       = findViewById(R.id.glSurfaceView)
        tvStatus     = findViewById(R.id.tvStatus)
        tvSpeed      = findViewById(R.id.tvSpeed)
        tvFps        = findViewById(R.id.tvFps)
        tvHint       = findViewById(R.id.tvHint)
        btnPlaceCar  = findViewById(R.id.btnPlaceCar)
        btnThrottle  = findViewById(R.id.btnThrottle)
        btnBrake     = findViewById(R.id.btnBrake)
        btnHandbrake = findViewById(R.id.btnHandbrake)
        btnReset     = findViewById(R.id.btnReset)
        joystick     = findViewById(R.id.joystick)
    }

    // Init engine on GL thread (requires GL context)
    private fun initEngineWhenReady() {
        glView.queueEvent {
            try {
                val ok = NativeEngine.onCreate(
                    applicationContext, this@MainActivity, assets)
                engineReady = ok
                runOnUiThread {
                    if (!ok) tvStatus.text = "AR init failed — install ARCore from apkmirror.com"
                }
            } catch (e: Exception) {
                runOnUiThread {
                    tvStatus.text = "Error: ${e.message?.take(60)}"
                }
            }
        }
    }

    private fun setupButtons() {
        btnPlaceCar.setOnClickListener  { safeNative { NativeEngine.onPlaceCar()  } }
        btnReset.setOnClickListener     { safeNative { NativeEngine.onResetCar()  } }

        btnThrottle.setOnTouchListener  { _, e ->
            safeNative { NativeEngine.onThrottle(e.action == android.view.MotionEvent.ACTION_DOWN) }
            true
        }
        btnBrake.setOnTouchListener     { _, e ->
            safeNative { NativeEngine.onBrake(e.action == android.view.MotionEvent.ACTION_DOWN) }
            true
        }
        btnHandbrake.setOnTouchListener { _, e ->
            safeNative { NativeEngine.onHandbrake(e.action == android.view.MotionEvent.ACTION_DOWN) }
            true
        }
    }

    private fun setupJoystick() {
        joystick.onMove = { x, y -> safeNative { NativeEngine.onJoystick(x, y) } }
    }

    // Safely dispatch to GL thread only when engine is ready
    private fun safeNative(block: () -> Unit) {
        if (engineReady) glView.queueEvent {
            try { block() } catch (_: Exception) {}
        }
    }

    private fun startHudLoop() {
        lifecycleScope.launch {
            while (isActive) {
                delay(150)
                if (!engineReady) continue
                try {
                    val state = NativeEngine.getGameState()
                    val speed = NativeEngine.getSpeedKmh()
                    val fps   = NativeEngine.getFps()
                    runOnUiThread { updateHud(state, speed, fps) }
                } catch (_: Exception) {}
            }
        }
    }

    private fun updateHud(state: Int, speed: Float, fps: Float) {
        tvFps.text   = "%.0f fps".format(fps)
        tvSpeed.text = "%.0f km/h".format(speed)
        when (state) {
            0 -> { tvStatus.text = "⏳ Initializing..."; tvHint.text = "" }
            1 -> {
                tvStatus.text = "📡 Scanning floor..."
                tvHint.text   = "Point camera at a flat surface"
                btnPlaceCar.visibility = View.GONE
                setDrivingUI(false)
            }
            2 -> {
                tvStatus.text = "✅ Floor detected!"
                tvHint.text   = "Tap PLACE CAR to start"
                btnPlaceCar.visibility = View.VISIBLE
                setDrivingUI(false)
            }
            3 -> {
                tvStatus.text = ""; tvHint.text = ""
                btnPlaceCar.visibility = View.GONE
                setDrivingUI(true)
            }
        }
    }

    private fun setDrivingUI(on: Boolean) {
        val v = if (on) View.VISIBLE else View.INVISIBLE
        listOf(joystick, btnThrottle, btnBrake,
               btnHandbrake, btnReset, tvSpeed).forEach { it.visibility = v }
    }

    override fun onResume() {
        super.onResume()
        setImmersiveMode()
        glView.onResume()
        if (engineReady) safeNative { NativeEngine.onResume() }
    }

    override fun onPause() {
        glView.onPause()
        if (engineReady) safeNative { NativeEngine.onPause() }
        super.onPause()
    }

    override fun onDestroy() {
        if (engineReady) safeNative { NativeEngine.onDestroy() }
        super.onDestroy()
    }

    private fun hasCameraPermission() =
        ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) ==
                PackageManager.PERMISSION_GRANTED

    private fun requestCameraPermission() =
        ActivityCompat.requestPermissions(this,
            arrayOf(Manifest.permission.CAMERA), CAM_PERM)

    override fun onRequestPermissionsResult(code: Int,
            perms: Array<out String>, results: IntArray) {
        super.onRequestPermissionsResult(code, perms, results)
        if (code == CAM_PERM &&
            results.firstOrNull() == PackageManager.PERMISSION_GRANTED)
            initEngineWhenReady()
        else
            Toast.makeText(this, "Camera permission required for AR", Toast.LENGTH_LONG).show()
    }

    @Suppress("DEPRECATION")
    private fun setImmersiveMode() {
        window.decorView.systemUiVisibility = (
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            or View.SYSTEM_UI_FLAG_FULLSCREEN
            or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
        )
    }
}
