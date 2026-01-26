cmake_minimum_required(VERSION 3.22.1)

project("android_app")

# Configurar OpenCV
set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/../../../../opencv-sdk/native/jni)
find_package(OpenCV REQUIRED)

# Verificación en logs
message(STATUS "OpenCV encontrado en: ${OpenCV_DIR}")
message(STATUS "OpenCV Include Dirs: ${OpenCV_INCLUDE_DIRS}")

include_directories(${OpenCV_INCLUDE_DIRS})

# Crear librería compartida
add_library(
        android_app
        SHARED
        native-lib.cpp
)

# Buscar librerías del sistema
find_library(log-lib log)
find_library(graphics-lib jnigraphics)

# Enlazar todas las librerías
target_link_libraries(
        android_app
        ${log-lib}
        ${graphics-lib}
        ${OpenCV_LIBS}
)