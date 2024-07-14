package com.example.nativelib

object NativeLib {
    // Used to load the 'nativelib' library on application startup.
    init {
        System.loadLibrary("native-lib")
    }

    /**
     * A native method that is implemented by the 'nativelib' native library,
     * which is packaged with this application.
     */
    @JvmStatic
    external fun stringFromJNI(): String

    @JvmStatic
    external fun ffmpegVersion(): String
}