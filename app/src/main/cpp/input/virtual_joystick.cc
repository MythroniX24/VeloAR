// input/virtual_joystick.cc
// The virtual joystick is implemented as a Kotlin View (VirtualJoystick.kt).
// C++ input state is updated via JNI in jni_bridge.cc -> InputManager.
// This file satisfies the CMake source list entry.
#include "input_manager.h"
