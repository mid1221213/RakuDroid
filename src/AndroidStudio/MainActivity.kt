package com.example.myapplication

import android.app.Activity
import android.os.Bundle
import com.example.myapplication.utils.extractAssets
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : Activity() {
    private var toPath : String = ""

    private external fun rakuEval(toEval: String): String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        toPath = filesDir.absolutePath

        eval_button.setOnClickListener {
            output_text.text = rakuEval(input_text.text.toString())
        }
        extract_button.setOnClickListener {
            extractAssets(toPath, this, true)
        }
    }
    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }
}
