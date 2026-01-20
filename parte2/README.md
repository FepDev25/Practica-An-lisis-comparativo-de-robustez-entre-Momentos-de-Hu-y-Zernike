# Parte 2: Shape Signature con FFT de Coordenadas Complejas

## 📁 Estructura de Directorios

Crea esta estructura antes de ejecutar:

```
parte2/
├── CMakeLists.txt
├── main.cpp
├── build/              # Directorio de compilación
└── data/
    ├── training/       # Corpus de entrenamiento (15 imágenes)
    │   ├── circle/
    │   │   ├── circle_1.png
    │   │   ├── circle_2.png
    │   │   ├── circle_3.png
    │   │   ├── circle_4.png
    │   │   └── circle_5.png
    │   ├── triangle/
    │   │   ├── triangle_1.png
    │   │   ├── triangle_2.png
    │   │   ├── triangle_3.png
    │   │   ├── triangle_4.png
    │   │   └── triangle_5.png
    │   └── square/
    │       ├── square_1.png
    │       ├── square_2.png
    │       ├── square_3.png
    │       ├── square_4.png
    │       └── square_5.png
    ├── testing/        # Dataset de prueba (30 imágenes)
    │   ├── circle/     # 10 círculos
    │   ├── triangle/   # 10 triángulos
    │   └── square/     # 10 cuadrados
    └── corpus.csv      # Se genera automáticamente
```

## 🚀 Cómo Compilar

```bash
# 1. Crear directorio de compilación
cd parte2
mkdir -p build
cd build

# 2. Configurar con CMake
cmake ..

# 3. Compilar
make

# Si hay errores, limpia y recompila:
# rm -rf * && cmake .. && make
```

## 🎯 Cómo Usar

### 1️⃣ Generar Corpus de Entrenamiento

Primero, dibuja/fotografía **5 imágenes de cada clase** (círculo, triángulo, cuadrado).
Guárdalas en `data/training/[clase]/`

Luego ejecuta:

```bash
./shape_app train
```

Esto procesará todas las imágenes y creará `data/corpus.csv` con los descriptores.

**Output esperado:**
```
========================================
Procesando: circle_1.png
========================================
✓ Contorno extraído: 423 puntos, área = 15234.5 px²
✓ Contorno interpolado: 423 → 1024 puntos
✓ Centroide calculado: (256.3, 198.7)
✓ Señal compleja construida: z(n) = (x-xc) + j(y-yc)
✓ FFT calculada: 1024 coeficientes
✓ Descriptor normalizado: 15 armónicos (F[0]=1234.5 descartado)
✅ Descriptor extraído exitosamente
...
✓ Corpus guardado: ../data/corpus.csv (15 ejemplos)
```

### 2️⃣ Evaluar Dataset de Prueba

Dibuja **10 imágenes de cada clase** en la app móvil (o descárgalas).
Guárdalas en `data/testing/[clase]/`

```bash
./shape_app test
```

**Output esperado:**
```
✓ Corpus cargado: ../data/corpus.csv (15 ejemplos)
✓ Real: circle | Predicho: circle | Distancia: 0.234
✓ Real: triangle | Predicho: triangle | Distancia: 0.187
✗ Real: square | Predicho: circle | Distancia: 0.456
...

📊 MATRIZ DE CONFUSIÓN:
           circle  triangle  square
circle     9       0         1
triangle   0       10        0
square     1       0         9

✅ ACCURACY: 93.33%
```

### 3️⃣ Clasificar una Imagen Individual

```bash
./shape_app classify ../data/testing/circle/test_circle.png
```

**Output:**
```
🎯 RESULTADO: circle (distancia: 0.234)
```

## 🧪 Explicación del Algoritmo

### Pipeline Matemático (según profesor):

1. **SACAR EL CONTORNO**
   - `findContours()` extrae el borde de la figura
   - Selecciona el contorno más grande

2. **INTERPOLACIÓN LINEAL a 1024 puntos**
   - Distribuye 1024 puntos uniformemente sobre el contorno
   - NO 64 puntos (como dice el profesor)

3. **CALCULAR COORDENADAS COMPLEJAS**
   - Centroide: `(xc, yc) = centro de masa`
   - Señal: `z(n) = (x(n) - xc) + j*(y(n) - yc)`
   - SI O SI usar este método (no distancia r(n))

4. **SACAR LA TRANSFORMADA DE FOURIER**
   - FFT convierte contorno → frecuencias
   - **ESTA ES LA FIRMA DE LA FIGURA**

5. **NORMALIZAR por |F(1)|**
   - `F[0]` = energía (NO se usa para comparar)
   - `F[1]` = primer armónico (normalización)
   - Descriptor final: `F'[k] = F[k] / |F[1]|` para k=1..15

6. **COMPARAR con distancia euclídea**
   - Menor distancia = más parecido
   - Clasificación: seleccionar el más cercano del corpus

## 🔧 Parámetros Importantes

```cpp
const int NUM_POINTS = 1024;      // Interpolación (según profesor)
const int NUM_HARMONICS = 15;     // Armónicos en el descriptor
```

Puedes ajustar `NUM_HARMONICS` si necesitas más/menos precisión:
- Menos armónicos (5-10) → más rápido, menos preciso
- Más armónicos (20-30) → más lento, más preciso

## ⚠️ Troubleshooting

**Error: "No se encontraron contornos"**
- La imagen debe tener fondo NEGRO y trazo BLANCO
- Prueba invertir: `cv::bitwise_not(img, img);`

**Error: "Contorno muy pequeño"**
- La figura debe ocupar al menos 100 píxeles²
- Usa imágenes más grandes

**Accuracy muy bajo**
- Verifica que las imágenes de entrenamiento sean buenas
- Aumenta el número de ejemplos por clase (de 5 a 10)
- Ajusta `NUM_HARMONICS`

## 📊 Checklist del Día 1 (Compañero A)

- [ ] ✅ Compilar el código sin errores
- [ ] ✅ Crear estructura de directorios `data/`
- [ ] ✅ Dibujar/fotografiar 15 figuras de entrenamiento (5 x 3 clases)
- [ ] ✅ Generar corpus (`./shape_app train`)
- [ ] ✅ Verificar que `corpus.csv` se creó correctamente
- [ ] ✅ Probar clasificación con 1-2 imágenes individuales
- [ ] ✅ Documentar parámetros y resultados

## 📝 Notas para tu Compañero (B)

**Lo que necesita de ti:**

1. **Archivo `corpus.csv`** con los descriptores de entrenamiento
2. **Formato de datos**: cada línea es `label,f1,f2,f3,...,f15`
3. **Función de clasificación**: Pasarle un descriptor → recibe label

**Cómo integrar en la app móvil:**

```cpp
// Pseudo-código para JNI (Android)
extern "C" JNIEXPORT jstring JNICALL
Java_com_tuapp_MainActivity_classifyShape(
    JNIEnv* env, jobject, jlong matAddr) {
    
    Mat& img = *(Mat*)matAddr;
    auto desc = extractShapeDescriptor(img);
    auto corpus = loadCorpus("corpus.csv");  // desde assets
    auto [label, distance] = classify(desc, corpus);
    
    return env->NewStringUTF(label.c_str());
}
```

## 🎯 Próximos Pasos (Día 2)

1. Adaptar código para JNI/wrapper
2. Integrar con app del Compañero B
3. Testing con 30 imágenes dibujadas en móvil
4. Generar matriz de confusión y calcular accuracy
5. Capturar screenshots de errores para el reporte
