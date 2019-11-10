package com.example.myapplication

import android.app.Application
import com.example.myapplication.utils.extractAssets

class MyApplication : Application() {
    private external fun rakuInit(dataDir: String): Void

    private var toPath : String = ""

    override fun onCreate() {
        super.onCreate()

        toPath = filesDir.absolutePath
        extractAssets(toPath, this, false)

        rakuInit(toPath)
    }

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }
}