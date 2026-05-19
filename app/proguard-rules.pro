# Keep all native method declarations
-keepclasseswithmembernames class * {
    native <methods>;
}
# Keep NativeEngine object (JNI target)
-keep class com.arracing.simulator.NativeEngine { *; }
-keep class com.arracing.simulator.MainActivity { *; }
