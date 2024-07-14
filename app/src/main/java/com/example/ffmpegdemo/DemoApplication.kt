package com.example.ffmpegdemo

import ando.file.BuildConfig
import ando.file.core.FileOperator
import android.app.Application
import android.util.Log
import android.widget.Toast
import kotlinx.coroutines.handleCoroutineException

/**
 *
 *
 * @author nanhua.sun
 * @since 2024/7/3
 */
class DemoApplication: Application() {
    override fun onCreate() {
        super.onCreate()
        FileOperator.init(this, BuildConfig.DEBUG)
        Thread.setDefaultUncaughtExceptionHandler { t, ex ->
            Toast.makeText(this, "程序遇到错误:" + ex.message, Toast.LENGTH_LONG).show()
            Log.e("DemoApplication", "ExceptionHandler: threadId=${t.id}, threadName=${t.name}",ex )
        }
    }
}