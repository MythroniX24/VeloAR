cmake_minimum_required(VERSION 3.22.1)
project(ar_racing_simulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── ARCore shared library ─────────────────────────────────────────
set(ARCORE_SO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../build/arcore-native/jni/${ANDROID_ABI}")
add_library(arcore_sdk_c SHARED IMPORTED)
set_target_properties(arcore_sdk_c PROPERTIES
    IMPORTED_LOCATION "${ARCORE_SO_DIR}/libarcore_sdk_c.so"
)

# ── GLM header-only ───────────────────────────────────────────────
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# ── EnTT header-only ──────────────────────────────────────────────
add_library(entt INTERFACE)
target_include_directories(entt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# ── Bullet Physics ────────────────────────────────────────────────
set(BULLET_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/bullet3/src)

file(GLOB_RECURSE BULLET_SOURCES
    ${BULLET_SRC}/LinearMath/*.cpp
    ${BULLET_SRC}/BulletCollision/BroadphaseCollision/*.cpp
    ${BULLET_SRC}/BulletCollision/CollisionDispatch/*.cpp
    ${BULLET_SRC}/BulletCollision/CollisionShapes/*.cpp
    ${BULLET_SRC}/BulletCollision/NarrowPhaseCollision/*.cpp
    ${BULLET_SRC}/BulletDynamics/ConstraintSolver/*.cpp
    ${BULLET_SRC}/BulletDynamics/Dynamics/*.cpp
    ${BULLET_SRC}/BulletDynamics/Vehicle/*.cpp
    ${BULLET_SRC}/BulletDynamics/Featherstone/*.cpp
    ${BULLET_SRC}/BulletDynamics/MLCPSolvers/*.cpp
)

add_library(bullet3 STATIC ${BULLET_SOURCES})
target_include_directories(bullet3 PUBLIC ${BULLET_SRC})
target_compile_options(bullet3 PRIVATE -O2 -fno-exceptions -fno-rtti -w)

# ── Main native library ───────────────────────────────────────────
add_library(ar_racing_native SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/jni/jni_bridge.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/core/engine.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/core/timer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/core/asset_manager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/core/logger.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/shader_manager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/mesh_renderer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/background_renderer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/plane_renderer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/shadow_map.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/texture_manager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ar/ar_session_manager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ar/plane_tracker.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ar/anchor_manager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/physics/physics_world.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/physics/vehicle_physics.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/physics/collision_debug.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/car/car_entity.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/car/car_mesh.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/car/wheel_mesh.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/scene/scene_manager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/scene/camera_controller.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/input/input_manager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/input/virtual_joystick.cc
)

target_include_directories(ar_racing_native PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${BULLET_SRC}
)

target_compile_options(ar_racing_native PRIVATE
    -O2 -fvisibility=hidden
    -Wno-unused-variable
    -Wno-unused-private-field
)

target_link_libraries(ar_racing_native
    android log EGL GLESv3
    arcore_sdk_c bullet3 glm entt
)
