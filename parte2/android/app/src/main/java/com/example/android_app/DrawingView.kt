package com.example.android_app

import android.content.Context
import android.graphics.*
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View

class DrawingView(context: Context, attrs: AttributeSet) : View(context, attrs) {

    private var mDrawPath: Path = Path()
    private var mDrawPaint: Paint = Paint()
    private var mCanvasPaint: Paint = Paint(Paint.DITHER_FLAG)

    // Este Bitmap es el que le pasaremos a C++ luego
    private var mCanvasBitmap: Bitmap? = null
    private var drawCanvas: Canvas? = null

    init {
        setupDrawing()
    }

    private fun setupDrawing() {
        mDrawPaint.color = Color.BLACK // Dibujamos en Negro
        mDrawPaint.isAntiAlias = true
        mDrawPaint.strokeWidth = 20f   // Grosor del trazo
        mDrawPaint.style = Paint.Style.STROKE
        mDrawPaint.strokeJoin = Paint.Join.ROUND
        mDrawPaint.strokeCap = Paint.Cap.ROUND
    }

    // Se llama cuando la vista cambia de tamaño (al iniciar)
    override fun onSizeChanged(w: Int, h: Int, oldw: Int, oldh: Int) {
        super.onSizeChanged(w, h, oldw, oldh)
        // Creamos el Bitmap con fondo blanco
        mCanvasBitmap = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888)
        drawCanvas = Canvas(mCanvasBitmap!!)
        drawCanvas?.drawColor(Color.WHITE) // Fondo blanco
    }

    override fun onDraw(canvas: Canvas) {
        canvas.drawBitmap(mCanvasBitmap!!, 0f, 0f, mCanvasPaint)
        canvas.drawPath(mDrawPath, mDrawPaint)
    }

    // Manejo del dedo (Touch Events)
    override fun onTouchEvent(event: MotionEvent): Boolean {
        val touchX = event.x
        val touchY = event.y

        when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                mDrawPath.moveTo(touchX, touchY)
            }
            MotionEvent.ACTION_MOVE -> {
                mDrawPath.lineTo(touchX, touchY)
            }
            MotionEvent.ACTION_UP -> {
                drawCanvas?.drawPath(mDrawPath, mDrawPaint)
                mDrawPath.reset()
            }
            else -> return false
        }
        invalidate() // Redibujar la vista
        return true
    }

    // Función pública para limpiar la pantalla
    fun clearCanvas() {
        drawCanvas?.drawColor(Color.WHITE)
        invalidate()
    }

    // Función para obtener el dibujo actual
    fun getBitmap(): Bitmap? {
        return mCanvasBitmap
    }
}