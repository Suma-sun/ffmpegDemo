package com.example.ffmpegdemo

import android.content.ActivityNotFoundException
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.core.content.FileProvider
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import androidx.navigation.fragment.findNavController
import com.example.ffmpegdemo.databinding.FragmentSecondBinding
import com.example.nativelib.DecodeTool
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.io.File


/**
 * A simple [Fragment] subclass as the second destination in the navigation.
 */
class SecondFragment : Fragment() {

    companion object {
        private const val PIC_DIR = "pic"
        private const val TAG = "DecodeFragment"
        private const val JPG_FILE_NAME = "1107.jpg"
    }

    private var _binding: FragmentSecondBinding? = null

    // This property is only valid between onCreateView and
    // onDestroyView.
    private val binding get() = _binding!!

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentSecondBinding.inflate(inflater, container, false)
        return binding.root

    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        val cacheRoot = MainActivity.getCacheDir(context) ?: return
        val pegCacheDir = File(cacheRoot,PIC_DIR)
        val outputFile = File(pegCacheDir, JPG_FILE_NAME)
        val copyFile = MainActivity.getVideoSource(context) ?: return

        if (!pegCacheDir.exists()) {
            pegCacheDir.mkdirs()
        }

        binding.buttonSecond.setOnClickListener {
            findNavController().navigate(R.id.action_SecondFragment_to_FirstFragment)
        }
        binding.btnSelect.setOnClickListener {
            lifecycleScope.launch(Dispatchers.IO) {
                DecodeTool.decodeMP4ToImage(copyFile.path,outputFile.path)
            }
        }
        binding.btnOpenFold.setOnClickListener {
            context?.let {
                openFold(it,cacheRoot)
            }
        }
    }

    private fun openFold(context: Context, cacheRoot:File) {
        val intent = Intent(Intent.ACTION_VIEW)
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
        if (Build.VERSION_CODES.N < Build.VERSION.SDK_INT) {
            val packageName = context.packageName
            // 获取应用内部存储目录的File对象
            val internalStorageDir = File(cacheRoot,"cache/$PIC_DIR")
            // 使用FileProvider获取文件的URI
            val fileUri =
                FileProvider.getUriForFile(context, "$packageName.provider", internalStorageDir)
            Log.i(TAG, "openFold 7.0+ > ${fileUri.path}")
            intent.setDataAndType(fileUri, "resource/folder")
            try {
                startActivity(Intent.createChooser(intent, "Choose a file explorer"))
            } catch (e: Exception) {
                Log.e(TAG, "openFold 7.0+ > ${fileUri.path}",e)
            }
        } else {
            if (cacheRoot.exists()) {
                val cachePath = cacheRoot.absolutePath
                intent.setDataAndType(Uri.parse("file://$cachePath/$PIC_DIR"), "resource/folder")
                try {
                    context.startActivity(Intent.createChooser(intent, "Choose a file explorer"))
                } catch (ex: ActivityNotFoundException) {
                    Toast.makeText(context, "Please install a file manager.", Toast.LENGTH_SHORT).show()
                }
            } else {
                Toast.makeText(context, "Cache directory not found.", Toast.LENGTH_SHORT).show()
            }
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}