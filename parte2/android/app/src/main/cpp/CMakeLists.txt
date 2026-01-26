cmake_minimum_required(VERSION 3.22.1)

project("android_app")

# ------------------------------------------------------------
# 1. CONFIGURACIÓN DE OPENCV
# ------------------------------------------------------------
# IMPORTANTE: Cambia esta ruta a donde descomprimiste el SDK originalmente.
# Apunta a la carpeta 'native/jni' dentro del SDK.
set(OpenCV_DIR "/home/felipep/Documentos/universidad/practica4/parte2/OpenCV-android-sdk/sdk/native/jni")

find_package(OpenCV REQUIRED)

# DEPURACIÓN: Verificamos en el log si encontró las rutas
message(STATUS "OpenCV encontrado en: ${OpenCV_DIR}")
message(STATUS "OpenCV Include Dirs: ${OpenCV_INCLUDE_DIRS}")

# ------------------------------------------------------------
# 2. INCLUIR CABECERAS (¡ESTO FALTABA!)
# ------------------------------------------------------------
# Esta línea le dice al compilador dónde buscar 'opencv2/core.hpp'
include_directories(${OpenCV_INCLUDE_DIRS})

# ------------------------------------------------------------
# 3. DEFINIR TU LIBRERÍA
# ------------------------------------------------------------
add_library(
        android_app
        SHARED
        native-lib.cpp)

find_library(
        log-lib
        log)

find_library(
        graphics-lib
        jnigraphics)

# 4. ENLAZAR TODO
target_link_libraries(
        android_app
        ${log-lib}
        ${graphics-lib}
        ${OpenCV_LIBS} # Esto enlaza el código binario (.so)
)