package com.example.nativelib

import android.view.Surface

/**
 * 解码工具
 *
 * @author nanhua.sun
 * @since 2024/6/25
 */
object DecodeTool {

    init {
        System.loadLibrary("native-lib")
    }

    interface ProgressCallback{
        fun invoke(progress:Int)
    }

    external fun decodeMP4ToImage2(inputFilePath:String, path: String)

    /**
     * 播放指定文件的视频
     * @param fileSize 文件大小
     * @param callback 回传当前进度0-100
     * @return 0 success， other fail
     */
    external fun startVideoPlay(path:String, surface: Surface, fileSize:Long, callback: ProgressCallback):Int

    /**
     * 停止播放
     */
    external fun stopPlay()

    /**
     * 播放指定文件的音频
     */
    external fun startPlayAudio(path:String, fileSize:Long, callback: ProgressCallback):Int
}