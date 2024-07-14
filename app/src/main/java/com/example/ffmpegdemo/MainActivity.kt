package com.example.ffmpegdemo

import android.Manifest
import android.content.Context
import android.os.Build
import android.os.Bundle
import android.util.Log
import com.google.android.material.snackbar.Snackbar
import androidx.appcompat.app.AppCompatActivity
import androidx.navigation.findNavController
import androidx.navigation.ui.AppBarConfiguration
import androidx.navigation.ui.navigateUp
import androidx.navigation.ui.setupActionBarWithNavController
import android.view.Menu
import android.view.MenuItem
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import com.example.ffmpegdemo.databinding.ActivityMainBinding
import java.io.File
import java.nio.file.Files
import java.nio.file.StandardCopyOption

class MainActivity : AppCompatActivity() {

    companion object {
        private const val TAG = "MainActivity"
        const val MP4_DIR = "mp4"
        const val COPY_FILE_NAME = "1106.mp4"

        /**
         * 缓存的根目录
         */
        fun getCacheDir(context: Context?):File? {
            return context?.externalCacheDir
        }

        /**
         * 缓存mp4的目录
         */
        fun getMp4Dir(context: Context?):File? {
            val path = context?.externalCacheDir?.path?:return null
            return File(path,MP4_DIR)
        }

        /**
         * 获取mp4源文件
         */
        fun getVideoSource(context: Context?):File? {
            val dir = getMp4Dir(context) ?: return null
            return File(dir,COPY_FILE_NAME)
        }
    }

    private lateinit var appBarConfiguration: AppBarConfiguration
    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setSupportActionBar(binding.toolbar)

        val navController = findNavController(R.id.nav_host_fragment_content_main)
        appBarConfiguration = AppBarConfiguration(navController.graph)
        setupActionBarWithNavController(navController, appBarConfiguration)

        binding.fab.setOnClickListener { view ->
            Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                .setAction("Action", null).show()
        }

        registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) {
            if (it.containsValue(false)) {
                Toast.makeText(this, "權限未同意", Toast.LENGTH_SHORT).show()
            } else {
                copySource()
            }
        }.launch(
            arrayOf(
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.WRITE_EXTERNAL_STORAGE
            )
        )
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        // Inflate the menu; this adds items to the action bar if it is present.
        menuInflater.inflate(R.menu.menu_main, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        return when (item.itemId) {
            R.id.action_settings -> true
            else -> super.onOptionsItemSelected(item)
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        val navController = findNavController(R.id.nav_host_fragment_content_main)
        return navController.navigateUp(appBarConfiguration)
                || super.onSupportNavigateUp()
    }

    /**
     * 从assets中提取文件到应用外部缓存目录
     */
    private fun copySource() {
        val mp4CacheDir = getMp4Dir(this) ?: return
        val copyFile = getVideoSource(this) ?: return
        if (!mp4CacheDir.exists()) {
            mp4CacheDir.mkdirs()
        }

        if (!copyFile.exists()) {
            Log.i(TAG, "btnSelect: before copyFile")
            try {
                copyFile.mkdirs()
                copyFile.createNewFile()
                val inputStream = resources.assets.open(COPY_FILE_NAME)
                Files.copy(inputStream,copyFile.toPath(), StandardCopyOption.REPLACE_EXISTING)
                Log.i(TAG, "btnSelect: copyFile success")
            } catch (e: Exception) {
                Log.e(TAG, "btnSelect: cpoy ", e)
            }
        } else {
            Log.i(TAG, "btnSelect: exists mp4 file ${copyFile.path}")
        }
    }
}