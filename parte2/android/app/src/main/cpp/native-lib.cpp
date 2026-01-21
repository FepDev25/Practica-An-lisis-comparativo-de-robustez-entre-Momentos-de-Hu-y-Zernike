#include <jni.h>
#include <string>
#include <android/bitmap.h> // librería para manejar Bitmaps
#include <android/log.h>

// Definir macros para logs (para verlos en Logcat)
#define LOG_TAG "JNI_PART"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_android_1app_MainActivity_classifyImage(
        JNIEnv* env,
        jobject /* this */,
        jobject bitmap) { // recibir el Bitmap

    AndroidBitmapInfo info;
    void* pixels;
    int ret;

    // obtener información del Bitmap (ancho, alto, formato)
    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGE("Error consiguiendo info del bitmap: %d", ret);
        return env->NewStringUTF("Error JNI: Info");
    }

    // verificación de formato (Debe ser RGBA_8888 o similar)
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGE("Formato de Bitmap no soportado (se requiere RGBA_8888)");
        return env->NewStringUTF("Error JNI: Formato");
    }

    // bloquear los píxeles para acceder a ellos desde C++
    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("Error bloqueando píxeles: %d", ret);
        return env->NewStringUTF("Error JNI: Lock");
    }

    LOGI("Imagen recibida: %dx%d", info.width, info.height);

    // aqui se llamara a la futura funcion

    // desbloquear píxeles
    AndroidBitmap_unlockPixels(env, bitmap);

    // devolver un string dummy por ahora para probar
    std::string hello = "Puente JNI Exitoso (" + std::to_string(info.width) + "x" + std::to_string(info.height) + ")";
    return env->NewStringUTF(hello.c_str());
}