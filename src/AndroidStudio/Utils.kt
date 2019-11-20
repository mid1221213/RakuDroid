package com.example.myapplication.utils

import android.content.Context
import android.content.res.AssetManager
import com.example.myapplication.MainActivity
import java.io.*
import java.util.logging.Logger

val my_log = Logger.getLogger(MainActivity::class.java.name)

fun extractAssets( toPath: String, ctx : Context,  force : Boolean) : Boolean {
    var ok = !force && File("$toPath/rakudroid").exists()

    if (!ok)
        ok = copyAssetFolder(
            ctx.assets,
            "rakudroid",
            "$toPath/rakudroid"
        )

    return ok
}

private fun copyAssetFolder(
    assetManager: AssetManager,
    fromAssetPath: String, toPath: String
): Boolean {
    my_log.warning("from=$fromAssetPath")
    my_log.warning("to=$toPath")
    val files = assetManager.list(fromAssetPath)
    var res = true
    for (file in files!!) {
//        my_log.warning("file from=$fromAssetPath/$file")
//        my_log.warning("file to=$toPath/$file")
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
    return try {
        val outFile = File(toPath)
        if (outFile.exists())
            outFile.delete()
        out = FileOutputStream(outFile)
        copyFile(`in`, out)
        `in`.close()
        out.flush()
        out.close()
        true
    } catch (e: Exception) {
        e.printStackTrace()
        false
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
