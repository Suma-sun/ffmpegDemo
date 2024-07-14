package com.example.ffmpegdemo.decode

import ando.file.core.FileUri
import android.graphics.PixelFormat
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import com.example.ffmpegdemo.MainActivity
import com.example.ffmpegdemo.databinding.FragmentDecodePlayBinding
import com.example.nativelib.DecodeTool
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.io.File

/**
 * 解码并播放
 *
 * @author nanhua.sun
 * @since 2024/7/1
 */
class DecodeVideoPlayFragment: Fragment() {

    companion object{
        private const val TAG = "DecodePlayFragment"
    }

    private lateinit var binding:FragmentDecodePlayBinding

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        binding = FragmentDecodePlayBinding.inflate(inflater)
        return binding.root
    }


    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        binding.tvPlay.setOnClickListener {
            val ctx = context?:return@setOnClickListener
            val file = File(ctx.externalCacheDir?: return@setOnClickListener,"${MainActivity.MP4_DIR}/${MainActivity.COPY_FILE_NAME}")
            val uri = FileUri.getUriByFile(file,true)?:return@setOnClickListener
            val path = uri.path
            Log.i(TAG, "tvPlay setOnClickListener: path=$path")
            binding.seekBar.max = 100
            lifecycleScope.launch(Dispatchers.IO) {
                DecodeTool.startVideoPlay(
                    path?:return@launch,
                    binding.surfaceView.holder.surface?:return@launch,
                    file.length(), callback = object :DecodeTool.ProgressCallback {
                        override fun invoke(progress: Int) {
                            binding.seekBar.post{
                                binding.seekBar.progress = progress
                            }
                            Log.i(TAG, "invoke: $progress")
                        }
                    }
                )
            }
        }
        binding.tvStop.setOnClickListener {
            DecodeTool.stopPlay()
        }
        binding.surfaceView.holder.setFormat(PixelFormat.RGBA_8888)
    }

    override fun onPause() {
        super.onPause()
        DecodeTool.stopPlay()
    }


}