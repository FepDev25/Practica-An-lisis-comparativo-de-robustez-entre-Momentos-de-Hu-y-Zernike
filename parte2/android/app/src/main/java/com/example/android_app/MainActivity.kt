package com.example.android_app

import android.graphics.Bitmap
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.example.android_app.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Configurar botón Borrar
        binding.btnClear.setOnClickListener {
            binding.drawingView.clearCanvas()
            binding.tvResult.text = "Lienzo limpio"
        }

        // Configurar botón Clasificar
        binding.btnClassify.setOnClickListener {
            val bitmap = binding.drawingView.getBitmap()
            if (bitmap != null) {
                // 2. LLAMAMOS A C++ AQUÍ
                val result = classifyImage(bitmap)
                binding.tvResult.text = "Resultado: $result"
            } else {
                binding.tvResult.text = "Error: Lienzo vacío"
            }
        }

    }
    external fun classifyImage(bitmap: Bitmap): String

    companion object {
        init {
            System.loadLibrary("android_app")
        }
    }
}