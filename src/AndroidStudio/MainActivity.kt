package com.example.myapplication

import android.content.Context
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_main.*
import android.content.res.AssetManager
import android.util.Log
import java.io.*
import java.util.logging.Logger


class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val toPath = this.filesDir.getAbsolutePath()

        val ok = copyAssetFolder(
            this.assets,
            "rakudroid",
            toPath + "/rakudroid"
        )

        if (ok)
            sample_text.text = stringFromJNI(toPath)
        else
            sample_text.text = "** FAILED ** " + toPath
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(dataDir: String): String

    companion object {

        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }
    val my_log = Logger.getLogger(MainActivity::class.java.name)

    private fun copyAssetFolder(
        assetManager: AssetManager,
        fromAssetPath: String, toPath: String
    ): Boolean {
        my_log.warning("from=$fromAssetPath")
        my_log.warning("to=$toPath")
        val files = assetManager.list(fromAssetPath)
        var res = true
        for (file in files!!) {
            my_log.warning("file from=$fromAssetPath/$file")
            my_log.warning("file to=$toPath/$file")
            File(toPath).mkdirs()
            val `in`: InputStream
            var isDir : Boolean = false
            try {
                `in` = assetManager.open("$fromAssetPath/$file")
                res = copyAsset(
                    `in`,
                    "$toPath/$file"
                ) and res
            } catch (e: Exception) {
                isDir = true
            }
            if (isDir)
                res = copyAssetFolder(
                    assetManager,
                    "$fromAssetPath/$file",
                    "$toPath/$file"
                ) and res
        }
        return res
    }

    private fun copyAsset(
        `in`: InputStream, toPath: String
    ): Boolean {
        val out: OutputStream
        try {
            out = FileOutputStream(toPath)
            copyFile(`in`, out)
            `in`.close()
            out.flush()
            out.close()
            return true
        } catch (e: Exception) {
            e.printStackTrace()
            return false
        }

    }

    @Throws(IOException::class)
    private fun copyFile(`in`: InputStream, out: OutputStream) {
        val buffer = ByteArray(1024)
        var read: Int
        read = `in`.read(buffer)
        while (read != -1) {
            out.write(buffer, 0, read)
            read = `in`.read(buffer)
        }
    }
}
