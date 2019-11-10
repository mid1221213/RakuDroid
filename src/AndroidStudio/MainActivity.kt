package com.example.myapplication

import android.app.Activity
import android.content.res.AssetManager
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_main.*
import java.io.*
import java.util.logging.Logger


class MainActivity : Activity() {

    var toPath : String = ""

    external fun rakuInit(dataDir: String): Void
    external fun rakuEval(toEval: String): String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        toPath = filesDir.getAbsolutePath()

        rakuInit(toPath)

        eval_button.setOnClickListener {
            output_text.text = rakuEval(input_text.text.toString())
        }
        extract_button.setOnClickListener {
            extractAssets(true)
        }
    }

    fun extractAssets(force : Boolean) {
        var ok = !force && File(toPath + "/rakudroid").exists()

        if (!ok)
            ok = copyAssetFolder(
                assets,
                "rakudroid",
                toPath + "/rakudroid"
            )

        output_text.text = if (ok) "OK" else "ERROR"
    }

    override fun onResume() {
        super.onResume()

        extractAssets(false)

        }



    companion object {
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
