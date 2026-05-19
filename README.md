# 🏎️ VeloAR

A **production-grade native Android augmented reality car driving simulator** built entirely in C++17 with OpenGL ES 3.0, ARCore NDK, and Bullet Physics. Kotlin is used only as a minimal Android bootstrap layer.

![GitHub Actions](https://github.com/YOUR_USERNAME/VeloAR/actions/workflows/android.yml/badge.svg)

---

## 📸 Screenshots

> _Place your in-device screenshots here_

| AR Floor Detection | Car Placed | Driving |
|---|---|---|
| ![scan](docs/screenshots/scan.png) | ![placed](docs/screenshots/placed.png) | ![drive](docs/screenshots/drive.png) |

---

## 🎮 Features

### Core
- Real-time ARCore camera passthrough
- Horizontal floor plane detection
- World-anchored 3D car placement
- Real-time physics at 120 Hz (Bullet btRaycastVehicle)
- OpenGL ES 3.0 renderer with PBR-ish shading
- Shadow-ready rendering pipeline

### Car Physics (Bullet btRaycastVehicle)
- 4-wheel suspension simulation
- RWD engine (rear-wheel drive)
- Independent steering interpolation
- Drift via handbrake + reduced rear friction
- Gravity, collision, surface friction
- Fixed-timestep at 240 Hz sub-steps

### Controls
- Virtual analog joystick (left thumb)
- Throttle / Brake / Drift buttons (right thumb)
- Single-finger camera orbit
- Pinch-to-zoom camera distance
- Reset car button

### Debug
- Live FPS counter
- Speed HUD (km/h)
- AR state display
- Bullet wireframe debug draw (BulletDebugDraw)

---

## 🏗️ Architecture

```
VeloAR/
├── app/src/main/
│   ├── cpp/
│   │   ├── core/           # Timer, AssetManager, Logger
│   │   ├── renderer/       # ShaderManager, MeshRenderer, BackgroundRenderer,
│   │   │                   #   PlaneRenderer, ShadowMap, TextureManager
│   │   ├── ar/             # ARSessionManager, PlaneTracker, AnchorManager
│   │   ├── physics/        # PhysicsWorld, VehiclePhysics, CollisionDebug
│   │   ├── car/            # CarEntity, CarMesh, WheelMesh
│   │   ├── scene/          # SceneManager (game loop), CameraController
│   │   ├── input/          # InputManager, InputState (atomic)
│   │   ├── jni/            # jni_bridge.cc (Kotlin ↔ C++ interface)
│   │   └── third_party/    # GLM, EnTT, Bullet3 (src), stb_image
│   ├── kotlin/
│   │   └── com/arracing/simulator/
│   │       ├── MainActivity.kt          # Android lifecycle + HUD
│   │       ├── NativeEngine.kt          # JNI declarations
│   │       ├── ARRacingGLSurfaceView.kt # GLSurfaceView + touch camera
│   │       └── VirtualJoystick.kt       # Custom touch joystick View
│   └── assets/
│       └── shaders/        # (future: external GLSL shader files)
└── .github/workflows/
    └── android.yml         # CI: builds Debug + Release APK
```

### System Flow

```
[Kotlin UI] → touch events → [JNI Bridge] → [InputManager (atomic)]
                                                    ↓
[ARCore NDK] → frame update → [SceneManager] ← ────┘
    ↓               ↓               ↓
[CameraFeed]  [PlaneTracker]  [PhysicsWorld]
    ↓               ↓               ↓
[BackgroundRenderer] [PlaneRenderer] [VehiclePhysics (btRaycastVehicle)]
                                         ↓
                                   [CarEntity.Render()]
                                         ↓
                                   [MeshRenderer + ShadowMap]
```

---

## 🛠️ Build Instructions

### Prerequisites

| Tool | Version |
|------|---------|
| Android Studio | Hedgehog+ |
| Android NDK | 26.3.x |
| CMake | 3.22.1 |
| Min SDK | 24 (Android 7.0) |
| ARCore | Required |

### 1. Clone

```bash
git clone https://github.com/YOUR_USERNAME/VeloAR.git
cd VeloAR
```

### 2. Get stb_image.h

```bash
curl -o app/src/main/cpp/third_party/stb_image.h \
     https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
```

### 3. Build from Android Studio

1. Open project in Android Studio
2. Sync Gradle
3. Connect AR-capable device (USB debug on)
4. Run `app`

### 4. Build from command line

```bash
chmod +x gradlew
./gradlew assembleDebug
# APK: app/build/outputs/apk/debug/app-debug.apk
```

---

## 📱 Device Requirements

- Android 7.0+ (API 24)
- ARCore supported device ([full list](https://developers.google.com/ar/devices))
- OpenGL ES 3.0
- Camera

---

## 🔧 Key C++ Systems

### ARSessionManager
Wraps `ArSession_create`, `ArSession_update`, provides `ViewMatrix()` and `ProjectionMatrix()` matched to live camera pose. Plane detection mode: `AR_PLANE_FINDING_MODE_HORIZONTAL`.

### VehiclePhysics
Uses `btRaycastVehicle` with 4 wheels. RWD engine on wheels [2,3]. Suspension: stiffness=20, damping=2.3. Rear friction multiplied by 0.85 for drift. Steering smoothly interpolated at 2.5 rad/s.

### SceneManager (Game Loop)
State machine: `Initializing → SearchingPlane → PlacingCar → Driving`. Physics runs at fixed 120 Hz via timer accumulator. AR anchor pose applied as world-space offset to Bullet transforms.

### BackgroundRenderer
Draws ARCore OES camera texture as fullscreen quad before depth write. UV coordinates from `ArFrame_transformCoordinates2d`.

---

## 🤝 Third-Party Libraries

| Library | Version | License |
|---------|---------|---------|
| [ARCore SDK](https://github.com/google-ar/arcore-android-sdk) | 1.44.0 | Apache 2.0 |
| [Bullet Physics](https://github.com/bulletphysics/bullet3) | 3.25 | zlib |
| [GLM](https://github.com/g-truc/glm) | 1.0.1 | MIT |
| [EnTT](https://github.com/skypjack/entt) | 3.13 | MIT |
| [stb_image](https://github.com/nothings/stb) | master | MIT/Public Domain |

---

## 📄 License

MIT — see [LICENSE](LICENSE)
