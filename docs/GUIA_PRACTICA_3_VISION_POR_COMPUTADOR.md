# Guía de Práctica de Laboratorio

## Visión por Computador

**Carrera:** Computación
**Asignatura:** Visión por Computador
**Docente:** Ing. Vladimir Robles Bykbaev
**Período Lectivo:** Octubre 2025 – Febrero 2026
**Resolución:** CS N° 076-04-2016-04-20

---

## Información General

* **Número de práctica**

  * 3-1
  * 3-2

* **Título de la práctica**
  Análisis comparativo de robustez entre Momentos de Hu y Zernike, y reconocimiento de formas mediante Firmas Espectrales con FFT de Coordenadas Complejas.

---

## Objetivo General

Evaluar la robustez y eficiencia de descriptores globales para el reconocimiento de formas geométricas (círculo, triángulo y rectángulo) mediante el análisis de momentos ortogonales y la implementación de una firma de la figura basada en la transformada de Fourier de coordenadas complejas en un entorno móvil.

---

## Instrucciones

* Revisar el contenido teórico del tema.
* Profundizar los conocimientos revisando los libros guía, enlaces en los objetos de aprendizaje y documentación académica disponible en línea.
* Desarrollar un conjunto de scripts y programas que permitan el reconocimiento de formas usando:

  * Momentos de Hu
  * Momentos de Zernike
  * Firma de la figura (Shape Signature)
* La práctica se divide en tareas relacionadas con los conceptos anteriores.

---

## Actividades por Desarrollar

### Objetivos Específicos

* Evaluar la invarianza a la rotación, escala y ruido de los Momentos de Hu frente a los Momentos de Zernike.
* Implementar un sistema de reconocimiento de formas en tiempo real mediante Descriptores de Fourier (FFT) usando coordenadas complejas.
* Analizar el impacto del ruido sintético en la degradación de características globales.

---

## Parte 1

### Cuaderno de Kaggle – Análisis de Robustez (Hu vs. Zernike)

Desarrollar y publicar un cuaderno en **Kaggle (Python)** clonando el cuaderno:

* `UPS-Writing-Skills`

---

### Preprocesamiento y Aumento de Datos

1. Cargar imágenes de las clases:

   * Circle
   * Triangle
   * Rectangle

   Ejemplos disponibles en el dataset **UPS-Writing-Skills**.

   Dataset:
   [https://www.kaggle.com/datasets/adolfogavilanes/ups-writing-skills/data](https://www.kaggle.com/datasets/adolfogavilanes/ups-writing-skills/data)

2. Diseñar una función de **inyección de ruido**:

   * Ruido Gaussiano
   * Ruido Salt & Pepper
   * Niveles de intensidad:

     * Bajo
     * Medio
     * Alto

3. Generar variantes con rotaciones aleatorias entre **0° y 360°**.

---

### Extracción de Características

#### Momentos de Hu

* Calcular los 7 momentos.
* Aplicar transformación logarítmica para mitigar diferencias de orden de magnitud.

#### Momentos de Zernike

* Implementar la extracción usando un grado de polinomio **n = 8** para capturar detalles finos.

---

### Evaluación de Clasificación

* Entrenar un clasificador simple:

  * KNN o
  * SVM
* Generar una **matriz de confusión** para cada nivel de ruido.

---

## Parte 2

### Aplicación Móvil – Shape Signature (Coordenadas Complejas)

Crear una **aplicación móvil** usando una librería nativa en **C++**, donde:

* Se realicen todos los cálculos.
* Se referencie al corpus de entrenamiento.
* Al mostrar una imagen o dibujo, se indique el tipo de figura:

  * triángulo
  * círculo
  * cuadrado

La aplicación debe incluir:

* Área de dibujo de la figura.
* Botón (u otro componente) para ejecutar la clasificación.

---

### Preprocesamiento de Imágenes

* Conversión a blanco y negro.
* Trazo en color blanco.
* Fondo en color negro.
* Se permite:

  * Esqueletización
  * Relleno del área interior
* Aplicar técnicas adicionales para mejorar resultados.

---

### Referencia Recomendada

* [https://link.springer.com/chapter/10.1007/978-3-031-19647-8_22](https://link.springer.com/chapter/10.1007/978-3-031-19647-8_22)

---

### Implementación en C++ (Reconocimiento por Contorno)

Usar JNI para Android o Wrapper para iOS.

#### Variante Matemática – Coordenadas Complejas

El contorno se representa como una señal compleja:

```bash
s(n) = (x(n) - xc) + j(y(n) - yc)
```

Donde `(xc, yc)` es el centroide de la figura.

Esta representación conserva la relación espacial completa del contorno.

---

### Flujo en Tiempo Real

* **Segmentación**

  * Binarización mediante umbral adaptativo
* **Extracción de contornos**

  * `cv::findContours` para obtener el contorno externo
* **Transformada de Fourier**

  * Aplicar `cv::dft` a la señal compleja
* **Normalización**

  * Dividir coeficientes por `|F(1)|`
  * Eliminar fase para invarianza a rotación

---

### Consideraciones Adicionales

* Usar **Albumentations** para aumentar el dataset.
* Evaluar al menos **30 imágenes dibujadas** en el dispositivo móvil.
* Comparar descriptores usando:

  * Distancia Euclídea
  * Otras métricas vistas en clase

---

### Ejemplo de FFT en OpenCV (C++)

```cpp
cv::Mat input = cv::Mat(signature).reshape(1, 1);
input.convertTo(input, CV_32F);
cv::Mat complexI;
cv::dft(input, complexI, cv::DFT_COMPLEX_OUTPUT);
```

---

### Reporte Final

El reporte debe incluir:

* Código fuente de la solución.
* Matriz de confusión.
* Nivel de precisión del sistema.
* Ejemplos gráficos de formas mal clasificadas.

---

## Resultados Esperados

Al finalizar la práctica, el estudiante será capaz de:

* Evaluar la robustez de descriptores globales ante ruido y transformaciones geométricas.
* Comparar Momentos de Hu y Zernike mediante análisis en Kaggle.
* Implementar Descriptores de Fourier en aplicaciones móviles.
* Diseñar sistemas de visión robustos con validación cuantitativa.

---

## Conclusiones

* Los Momentos de Zernike presentan mayor robustez ante ruido debido a su base ortogonal.
* La FFT con coordenadas complejas es eficiente y adecuada para dispositivos móviles.
* El preprocesamiento es crítico para preservar la topología del contorno.
* La práctica integra teoría matemática con aplicaciones reales de visión artificial.

---

## Recomendaciones

* Ajustar el número de armónicos de Fourier para evitar pérdida de detalles o ruido excesivo.
* Aplicar filtros de mediana ante ruido Salt & Pepper.
* Probar métricas de similitud adicionales como Manhattan o correlación cruzada.
* Usar PCA para visualizar la separación de clases bajo ruido.

---

## Cierre

**Docente / Técnico Docente:**
Ing. Vladimir Robles Bykbaev

**Firma:**

---
