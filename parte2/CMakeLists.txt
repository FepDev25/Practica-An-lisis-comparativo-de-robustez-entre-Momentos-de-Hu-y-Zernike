cmake_minimum_required(VERSION 3.16)
project(ShapeSignature LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Buscar OpenCV automáticamente en ubicaciones estándar del sistema
# Funciona en cualquier máquina donde OpenCV esté instalado
find_package(OpenCV REQUIRED)

# Mostrar información útil durante la configuración
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")

add_executable(shape_app main.cpp)

# Enlazar con OpenCV (PRIVATE es buena práctica)
target_link_libraries(shape_app PRIVATE ${OpenCV_LIBS})