cmake_minimum_required(VERSION 3.22.1)

project("android_app")

# Configurar OpenCV
set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/../../../../opencv-sdk/native/jni)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        native-lib.cpp)

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
        ${OpenCV_LIBS}
)