cmake_minimum_required(VERSION 3.10)
project(ShapeSignature)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenCV_DIR "$ENV{HOME}/Documentos/universidad/universidad 7mo/vision por computador/opencv-dev/install/lib/cmake/opencv4")

# Buscar el paquete en la ruta indicada arriba
find_package(OpenCV REQUIRED)

# Incluir cabeceras
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(shape_app main.cpp)

# Enlazar con las librerías de OpenCV encontradas
target_link_libraries(shape_app ${OpenCV_LIBS})