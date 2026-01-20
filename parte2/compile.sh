#!/bin/bash
# Script de compilación rápida para Shape Signature

echo "Compilando Shape Signature..."

# Ir al directorio build
cd "$(dirname "$0")/build" || exit 1

# Limpiar compilación anterior si se pasa --clean
if [ "$1" == "--clean" ]; then
    echo "Limpiando compilación anterior..."
    rm -rf ./*
fi

# Configurar con CMake
echo "Configurando con CMake..."
cmake .. || exit 1

# Compilar
echo "Compilando..."
make -j$(nproc) || exit 1

echo ""
echo "Compilación exitosa!"
echo ""
echo "Comandos disponibles:"
echo "  ./build/shape_app train              - Generar corpus de entrenamiento"
echo "  ./build/shape_app test               - Evaluar dataset de prueba"
echo "  ./build/shape_app classify <imagen>  - Clasificar una imagen"
echo ""
