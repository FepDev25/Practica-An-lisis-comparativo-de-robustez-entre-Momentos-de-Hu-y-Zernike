# Análisis Comparativo de Descriptores de Forma y Clasificación en Tiempo Real

### Nombres: Felipe Peralta y Samantha Suquilanda

## Descripción

Este proyecto implementa y compara tres técnicas de reconocimiento de formas geométricas (círculo, triángulo, cuadrado):

1. **Momentos de Hu**: Invariantes a traslación, rotación y escala basados en momentos centrales normalizados
2. **Momentos de Zernike**: Polinomios ortogonales complejos sobre el círculo unitario
3. **Shape Signature con FFT**: Transformada de Fourier de coordenadas complejas del contorno

El trabajo se divide en dos partes: análisis comparativo de robustez (Hu vs Zernike) en Python, y clasificación en tiempo real mediante FFT en Android con C++/OpenCV.

## Estructura del Proyecto

```
.
├── parte1/                  # Análisis comparativo Hu vs Zernike
│   └── notebook.ipynb       # Jupyter Notebook con experimentos
├── parte2/                  # Aplicación Android + algoritmo FFT
│   ├── main.cpp             # Implementación escritorio (generación corpus)
│   ├── CMakeLists.txt       # Configuración compilación C++
│   └── android/             # Aplicación móvil
│       ├── app/
│       │   ├── src/main/
│       │   │   ├── cpp/
│       │   │   │   ├── native-lib.cpp      # JNI + pipeline FFT
│       │   │   │   └── CMakeLists.txt      # Configuración OpenCV NDK
│       │   │   ├── java/.../
│       │   │   │   ├── MainActivity.kt     # Interfaz principal
│       │   │   │   └── DrawingView.kt      # Canvas interactivo
│       │   │   ├── assets/
│       │   │   │   └── corpus.csv          # 80 descriptores entrenamiento
│       │   │   └── res/layout/
│       │   │       └── activity_main.xml   # UI Layout
│       │   └── build.gradle.kts
│       └── opencv-sdk/                     # OpenCV 4.8.0 Android SDK
└── docs/                    # Documentación y guías

```

## Parte 1: Análisis de Robustez (Python)

### Objetivo

Evaluar la invarianza a rotación, escala y ruido de Momentos de Hu vs Momentos de Zernike mediante:

- Inyección de ruido gaussiano y salt & pepper (bajo, medio, alto)
- Rotaciones aleatorias 0-360°
- Clasificación con KNN/SVM
- Matrices de confusión por nivel de ruido

### Tecnologías

- Python 3.x
- OpenCV
- NumPy
- scikit-learn
- mahotas (Zernike)
- Matplotlib/Seaborn

### Ejecución

```bash
cd parte1
jupyter notebook notebook.ipynb
```

## Parte 2: Clasificación en Tiempo Real (Android)

### Arquitectura

**Frontend (Kotlin):**
- `DrawingView`: Canvas personalizado con eventos táctiles para dibujar
- `MainActivity`: Coordinación UI y llamadas JNI

**Backend (C++ con OpenCV):**
1. Recepción de Bitmap desde Java
2. Preprocesamiento: binarización adaptativa, morfología
3. Extracción de contorno principal
4. Interpolación lineal a 1024 puntos
5. Construcción señal compleja: `z(n) = (x(n) - xc) + j(y(n) - yc)`
6. FFT con `cv::dft`
7. Normalización: `F'[k] = |F[k]| / |F[1]|` (15 armónicos)
8. Clasificación por distancia euclídea contra corpus
9. Traducción resultado a español

### Pipeline FFT

```
Bitmap → Mat → Binarización → findContours → Contorno[N puntos]
                                                  ↓
Círculo ← Clasificación ← Normalización ← FFT ← Interpolación[1024]
Triángulo                                        ↓
Cuadrado                                    Señal Compleja
```

### Tecnologías

- Android Studio Ladybug 2024.2.1
- Kotlin 1.9.0
- OpenCV 4.8.0 (Native SDK)
- NDK 27.0.12077973
- CMake 3.22.1
- C++17
- JNI (Java Native Interface)

### Requisitos Android

- Android SDK API 34
- NDK instalado
- MinSdk 24 (Android 7.0+)

### Compilación

```bash
cd parte2/android
# Abrir en Android Studio
# Build → Clean Project
# Build → Make Project
# Run
```

### Generación del Corpus

El corpus contiene 80 descriptores FFT normalizados (20 círculos, 20 triángulos, 40 cuadrados):

```bash
cd parte2
mkdir -p build && cd build
cmake ..
make
./shape_recognition
# Genera corpus.csv → copiar a android/app/src/main/assets/
```

## Resultados

### Parte 1: Hu vs Zernike
- **Zernike** demuestra mayor robustez a ruido de alta intensidad
- **Hu** presenta degradación significativa con ruido salt & pepper
- Ambos mantienen invarianza a rotación
- Ver notebook para matrices de confusión detalladas

### Parte 2: FFT en Android
- **Precisión**: 93.33% en escritorio (círculo/triángulo: 100%, cuadrado: 80%)
- **Tiempo de clasificación**: <100ms en dispositivo (pruebas en emulador)
- **Invarianzas**: Traslación (centroide), rotación (normalización fase), escala (normalización por F[1])

## Metodología Científica

1. **Preprocesamiento consistente**: Umbral adaptativo, morfología de cierre/apertura
2. **Interpolación equidistante**: 1024 puntos garantizan uniformidad espectral
3. **Coordenadas complejas**: Preservan información geométrica 2D
4. **Normalización espectral**: División por componente fundamental elimina dependencia escala
5. **Clasificador no paramétrico**: Distancia euclídea, extensible a k-NN

## Referencias

- Zhang, D., & Lu, G. (2004). Shape-based image retrieval using generic Fourier descriptor. *Signal Processing: Image Communication*.
- Guía Práctica 3: Visión por Computador (Universidad Politécnica Salesiana)
- OpenCV Documentation: Structural Analysis and Shape Descriptors
- [Springer Chapter: Shape Recognition](https://link.springer.com/chapter/10.1007/978-3-031-19647-8_22)


