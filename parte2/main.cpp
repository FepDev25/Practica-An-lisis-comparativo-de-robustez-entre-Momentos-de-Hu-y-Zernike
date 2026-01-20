/**
 * PRÁCTICA 3-2: Shape Signature con FFT de Coordenadas Complejas
 * 
 * Pipeline según anotaciones del profesor:
 * 1. SACAR EL CONTORNO (findContours)
 * 2. INTERPOLACIÓN LINEAL a 1024 puntos (NO 64)
 * 3. CALCULAR coordenadas complejas z = (x-xc) + j(y-yc)
 * 4. SACAR LA TRANSFORMADA DE FOURIER → Esta es la FIRMA
 * 5. NORMALIZAR por |F(1)| (F(0) = energía, NO se usa para comparar)
 * 6. COMPARAR con distancia euclídea → menor distancia = más parecido
 * 
 * Autores: [Tu nombre] & [Compañero]
 * Fecha: Enero 2026
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <filesystem>

using namespace cv;
using namespace std;

// CONSTANTES GLOBALES

const int NUM_POINTS = 1024;        // Interpolación a 1024 puntos
const int NUM_HARMONICS = 15;       // Número de armónicos para el descriptor
const string TRAIN_DIR = "../data/training/";  // Corpus de entrenamiento
const string TEST_DIR = "../data/testing/";    // Imágenes de prueba

// ESTRUCTURA: Descriptor de Forma

struct ShapeDescriptor {
    vector<float> features;    // Magnitudes normalizadas de FFT
    string label;              // "circle", "triangle", "square"
    string filename;           // Nombre del archivo de origen
    
    ShapeDescriptor() {}
    ShapeDescriptor(const vector<float>& f, const string& l, const string& fn = "") 
        : features(f), label(l), filename(fn) {}
};

// PASO 1: PREPROCESAMIENTO Y EXTRACCIÓN DE CONTORNO

/**
 * Preprocesa la imagen y extrae el contorno principal.
 * 
 * Pipeline:
 * - Convertir a escala de grises (si es necesario)
 * - Binarización con umbral adaptativo (robusto a iluminación variable)
 * - Aplicar operaciones morfológicas para limpiar ruido
 * - Extraer contornos con findContours
 * - Seleccionar el contorno más grande (la figura principal)
 * 
 * @param image Imagen de entrada (puede ser color o gris)
 * @param contour Contorno extraído (output)
 * @return true si se encontró un contorno válido
 */
bool extractContour(const Mat& image, vector<Point>& contour) {
    Mat gray, binary;
    
    // Convertir a escala de grises si es necesario
    if (image.channels() == 3) {
        cvtColor(image, gray, COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Binarización adaptativa (mejor que umbral fijo)
    // Se adapta a variaciones locales de iluminación
    adaptiveThreshold(gray, binary, 255, ADAPTIVE_THRESH_GAUSSIAN_C, 
                      THRESH_BINARY_INV, 11, 2);
    
    // Operaciones morfológicas para limpiar ruido
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
    morphologyEx(binary, binary, MORPH_CLOSE, kernel);  // Cerrar huecos pequeños
    morphologyEx(binary, binary, MORPH_OPEN, kernel);   // Eliminar ruido puntual
    
    // Extraer todos los contornos
    vector<vector<Point>> contours;
    findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    
    if (contours.empty()) {
        cerr << " No se encontraron contornos en la imagen" << endl;
        return false;
    }
    
    // Seleccionar el contorno más grande
    double maxArea = 0;
    int maxIdx = 0;
    for (size_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        if (area > maxArea) {
            maxArea = area;
            maxIdx = i;
        }
    }
    
    contour = contours[maxIdx];
    
    // Validación: área mínima
    if (maxArea < 100) {
        cerr << " Contorno muy pequeño (área < 100 píxeles)" << endl;
        return false;
    }
    
    cout << "✓ Contorno extraído: " << contour.size() << " puntos, área = " 
         << maxArea << " px²" << endl;
    
    return true;
}

// PASO 2: INTERPOLACIÓN LINEAL A 1024 PUNTOS

/**
 * Interpola el contorno a exactamente NUM_POINTS puntos.
 * 
 * ¿Por qué interpolar?
 * - FFT requiere señales de longitud fija para comparar
 * - 1024 puntos captura suficientes detalles de la forma
 * - Potencia de 2 → FFT más eficiente
 * 
 * Método:
 * - Calcular longitud total del contorno (perímetro)
 * - Distribuir NUM_POINTS uniformemente a lo largo del contorno
 * - Interpolar linealmente entre puntos originales
 * 
 * @param contour Contorno original (tamaño variable)
 * @return Contorno interpolado (exactamente NUM_POINTS puntos)
 */
vector<Point2f> interpolateContour(const vector<Point>& contour) {
    int n = contour.size();
    
    if (n < 3) {
        cerr << " Contorno con muy pocos puntos: " << n << endl;
        return vector<Point2f>();
    }
    
    // Calcular longitud acumulada del contorno
    vector<float> cumulativeLength(n);
    cumulativeLength[0] = 0.0f;
    
    for (int i = 1; i < n; i++) {
        float dx = contour[i].x - contour[i-1].x;
        float dy = contour[i].y - contour[i-1].y;
        float dist = sqrt(dx*dx + dy*dy);
        cumulativeLength[i] = cumulativeLength[i-1] + dist;
    }
    
    float totalLength = cumulativeLength[n-1];
    
    // Interpolar NUM_POINTS uniformemente distribuidos
    vector<Point2f> interpolated(NUM_POINTS);
    
    for (int i = 0; i < NUM_POINTS; i++) {
        // Posición objetivo en el contorno
        float targetLength = (totalLength * i) / NUM_POINTS;
        
        // Buscar el segmento que contiene esta posición
        int idx = 0;
        while (idx < n-1 && cumulativeLength[idx+1] < targetLength) {
            idx++;
        }
        
        // Interpolar linealmente entre contour[idx] y contour[idx+1]
        if (idx < n-1) {
            float segmentLength = cumulativeLength[idx+1] - cumulativeLength[idx];
            float t = (targetLength - cumulativeLength[idx]) / segmentLength;
            
            interpolated[i].x = (1-t) * contour[idx].x + t * contour[idx+1].x;
            interpolated[i].y = (1-t) * contour[idx].y + t * contour[idx+1].y;
        } else {
            interpolated[i] = contour[idx];
        }
    }
    
    cout << "✓ Contorno interpolado: " << contour.size() 
         << " → " << NUM_POINTS << " puntos" << endl;
    
    return interpolated;
}

// PASO 3: CALCULAR CENTROIDE

/**
 * Calcula el centroide (centro de masa) del contorno.
 * 
 * Fórmula:
 *   xc = (1/N) * Σ x(i)
 *   yc = (1/N) * Σ y(i)
 * 
 * El centroide se usa para centrar la señal compleja, logrando:
 * - Invarianza a traslación
 * - El componente DC (F[0]) representa solo la energía
 * 
 * @param contour Contorno de entrada
 * @return Centroide (xc, yc)
 */
Point2f calculateCentroid(const vector<Point2f>& contour) {
    float sumX = 0, sumY = 0;
    
    for (const auto& pt : contour) {
        sumX += pt.x;
        sumY += pt.y;
    }
    
    Point2f centroid(sumX / contour.size(), sumY / contour.size());
    
    cout << "✓ Centroide calculado: (" << centroid.x << ", " 
         << centroid.y << ")" << endl;
    
    return centroid;
}

// PASO 4: CONSTRUIR SEÑAL COMPLEJA (COORDENADAS COMPLEJAS)

/**
 * Construye la señal compleja centrada en el centroide.
 * 
 * IMPORTANTE (según el profesor):
 * - SI O SI se debe usar el método de COORDENADAS COMPLEJAS
 * - NO usar distancia r(n) (método del ejercicio preparatorio)
 * 
 * Fórmula:
 *   s(n) = (x(n) - xc) + j*(y(n) - yc)
 * 
 * Donde:
 * - (x(n), y(n)) son las coordenadas del punto n
 * - (xc, yc) es el centroide
 * - j es la unidad imaginaria
 * 
 * Esta representación:
 * - Preserva toda la información espacial 2D
 * - La parte real = desplazamiento horizontal
 * - La parte imaginaria = desplazamiento vertical
 * 
 * @param contour Contorno interpolado
 * @param centroid Centroide del contorno
 * @return Señal compleja en formato OpenCV (Mat con 2 canales: real, imag)
 */
Mat buildComplexSignal(const vector<Point2f>& contour, const Point2f& centroid) {
    int n = contour.size();
    
    // OpenCV representa números complejos como Mat de 2 canales
    // Canal 0 = parte real, Canal 1 = parte imaginaria
    Mat complexSignal(n, 1, CV_32FC2);
    
    for (int i = 0; i < n; i++) {
        float real = contour[i].x - centroid.x;  // x - xc
        float imag = contour[i].y - centroid.y;  // y - yc
        
        complexSignal.at<Vec2f>(i, 0) = Vec2f(real, imag);
    }
    
    cout << "✓ Señal compleja construida: z(n) = (x-xc) + j(y-yc)" << endl;
    
    return complexSignal;
}

// PASO 5: TRANSFORMADA DE FOURIER (FFT)

/**
 * Aplica la Transformada Discreta de Fourier (DFT/FFT).
 * 
 * ESTA ES LA FIRMA DE LA FIGURA
 * 
 * La FFT descompone la señal en componentes de frecuencia:
 * - F[0] = componente DC (energía total, NO se usa para comparar)
 * - F[1] = primer armónico (fundamental, se usa para normalizar)
 * - F[2], F[3], ... = armónicos superiores (detalles de la forma)

 * @param complexSignal Señal compleja de entrada
 * @param magnitudes Magnitudes de los coeficientes de Fourier (output)
 */
void computeFFT(const Mat& complexSignal, vector<float>& magnitudes) {
    Mat dftOutput;
    
    // Aplicar DFT (Discrete Fourier Transform)
    // DFT_COMPLEX_OUTPUT: salida en formato complejo (2 canales)
    dft(complexSignal, dftOutput, DFT_COMPLEX_OUTPUT);
    
    // Separar parte real e imaginaria
    vector<Mat> planes(2);
    split(dftOutput, planes); // planes[0] = real, planes[1] = imag
    
    // Calcular magnitudes
    Mat mag;
    magnitude(planes[0], planes[1], mag);
    
    // Convertir a vector para facilitar manipulación
    magnitudes.clear();
    for (int i = 0; i < mag.rows; i++) {
        magnitudes.push_back(mag.at<float>(i, 0));
    }
    
    cout << "✓ FFT calculada: " << magnitudes.size() << " coeficientes" << endl;
}

// PASO 6: NORMALIZACIÓN (INVARIANZA A ESCALA)

/**
 * Normaliza los coeficientes de Fourier para invarianza a escala.

 * - El primer componente F[0] es solo ENERGÍA DE LA SEÑAL
 * - Se usa para LOCALIZAR los demás coeficientes
 * 
 * Resultado:
 * - F'[1] = 1.0 siempre (normalizado)
 * - F'[2], F'[3], ... son relativos al fundamental
 * - Invariante a escala (figuras grandes/pequeñas tienen mismo descriptor)
 * 
 * @param magnitudes Magnitudes de FFT sin normalizar
 * @return Vector de características normalizado (NUM_HARMONICS valores)
 */
vector<float> normalizeDescriptor(const vector<float>& magnitudes) {
    if (magnitudes.size() < 2) {
        cerr << " Muy pocos coeficientes de Fourier" << endl;
        return vector<float>(NUM_HARMONICS, 0.0f);
    }
    
    // F[0] = componente DC (energía)
    float dc = magnitudes[0];
    
    // F[1] = primer armónico (fundamental), usado para normalizar
    float fundamental = magnitudes[1];
    
    if (fundamental < 1e-5) {
        cerr << "Fundamental muy pequeño, posible error en la señal" << endl;
        return vector<float>(NUM_HARMONICS, 0.0f);
    }
    
    // Construir descriptor normalizado
    vector<float> descriptor;
    
    // Empezamos desde k=1 (saltamos F[0] = energía)
    for (int k = 1; k <= NUM_HARMONICS && k < magnitudes.size(); k++) {
        float normalized = magnitudes[k] / fundamental;
        descriptor.push_back(normalized);
    }
    
    // Rellenar con ceros si no hay suficientes armónicos
    while (descriptor.size() < NUM_HARMONICS) {
        descriptor.push_back(0.0f);
    }
    
    cout << "✓ Descriptor normalizado: " << descriptor.size() 
         << " armónicos (F[0]=" << dc << " descartado)" << endl;
    
    return descriptor;
}

// FUNCIÓN PRINCIPAL: EXTRAER DESCRIPTOR COMPLETO

/**
 * Pipeline completo de extracción de Shape Signature.
 * 
 * Pasos:
 * 1. Sacar el contorno
 * 2. Interpolar a 1024 puntos
 * 3. Calcular centroide
 * 4. Construir señal compleja z = (x-xc) + j(y-yc)
 * 5. Aplicar FFT → FIRMA
 * 6. Normalizar por |F[1]|
 * 
 * @param image Imagen de entrada
 * @param label Etiqueta de la clase (opcional)
 * @param filename Nombre del archivo (opcional)
 * @return Descriptor de forma
 */
ShapeDescriptor extractShapeDescriptor(const Mat& image, 
                                       const string& label = "", 
                                       const string& filename = "") {
    cout << "\n========================================" << endl;
    cout << "Procesando: " << (filename.empty() ? "imagen" : filename) << endl;
    cout << "========================================" << endl;
    
    // PASO 1: Extraer contorno
    vector<Point> contour;
    if (!extractContour(image, contour)) {
        return ShapeDescriptor();
    }
    
    // PASO 2: Interpolar a 1024 puntos
    vector<Point2f> interpolated = interpolateContour(contour);
    if (interpolated.empty()) {
        return ShapeDescriptor();
    }
    
    // PASO 3: Calcular centroide
    Point2f centroid = calculateCentroid(interpolated);
    
    // PASO 4: Construir señal compleja
    Mat complexSignal = buildComplexSignal(interpolated, centroid);
    
    // PASO 5: FFT (FIRMA)
    vector<float> magnitudes;
    computeFFT(complexSignal, magnitudes);
    
    // PASO 6: Normalizar
    vector<float> descriptor = normalizeDescriptor(magnitudes);
    
    cout << "Descriptor extraído exitosamente" << endl;
    
    return ShapeDescriptor(descriptor, label, filename);
}

// PASO 7: COMPARACIÓN (DISTANCIA EUCLÍDEA)

/**
 * Calcula la distancia euclídea entre dos descriptores.
 * - Mientras más parecidas sean las formas, MÁS PEQUEÑO el valor de la distancia
 * 

 * @param d1 Primer descriptor
 * @param d2 Segundo descriptor
 * @return Distancia euclídea (0 = idénticos, mayor = más diferentes)
 */
float euclideanDistance(const vector<float>& d1, const vector<float>& d2) {
    if (d1.size() != d2.size()) {
        cerr << "Descriptores de diferente tamaño" << endl;
        return 1e9;  // Distancia infinita
    }
    
    float sum = 0.0f;
    for (size_t i = 0; i < d1.size(); i++) {
        float diff = d1[i] - d2[i];
        sum += diff * diff;
    }
    
    return sqrt(sum);
}

/**
 * Clasifica una imagen comparándola con el corpus de entrenamiento.
 * 
 * Método:
 * - Calcular distancia a TODOS los ejemplos del corpus
 * - Seleccionar el más cercano (distancia mínima)
 * - Retornar su etiqueta
 * 
 * @param testDescriptor Descriptor de la imagen de prueba
 * @param trainingSet Corpus de entrenamiento
 * @return Etiqueta predicha y distancia mínima
 */
pair<string, float> classify(const ShapeDescriptor& testDescriptor, 
                             const vector<ShapeDescriptor>& trainingSet) {
    if (trainingSet.empty()) {
        cerr << " Corpus de entrenamiento vacío" << endl;
        return {"unknown", 1e9};
    }
    
    string bestLabel = "unknown";
    float minDistance = 1e9;
    
    for (const auto& train : trainingSet) {
        float dist = euclideanDistance(testDescriptor.features, train.features);
        
        if (dist < minDistance) {
            minDistance = dist;
            bestLabel = train.label;
        }
    }
    
    return {bestLabel, minDistance};
}

// UTILIDADES: CARGAR/GUARDAR CORPUS


void saveCorpus(const vector<ShapeDescriptor>& corpus, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << " No se pudo crear archivo: " << filename << endl;
        return;
    }
    
    // Formato: label,feature1,feature2,...,featureN
    for (const auto& desc : corpus) {
        file << desc.label;
        for (float f : desc.features) {
            file << "," << f;
        }
        file << "\n";
    }
    
    file.close();
    cout << "✓ Corpus guardado: " << filename << " (" 
         << corpus.size() << " ejemplos)" << endl;
}


vector<ShapeDescriptor> loadCorpus(const string& filename) {
    vector<ShapeDescriptor> corpus;
    ifstream file(filename);
    
    if (!file.is_open()) {
        cerr << " No se pudo abrir archivo: " << filename << endl;
        return corpus;
    }
    
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string label;
        getline(ss, label, ',');
        
        vector<float> features;
        string value;
        while (getline(ss, value, ',')) {
            features.push_back(stof(value));
        }
        
        corpus.push_back(ShapeDescriptor(features, label));
    }
    
    file.close();
    cout << "✓ Corpus cargado: " << filename << " (" 
         << corpus.size() << " ejemplos)" << endl;
    
    return corpus;
}


// FUNCIÓN PRINCIPAL: GENERAR CORPUS DE ENTRENAMIENTO

/**
 * Genera el corpus de entrenamiento procesando todas las imágenes en train_dir.
 * 
 * Estructura esperada:
 * data/training/
 *   ├── circle/
 *   │   ├── circle_1.png
 *   │   └── circle_2.png
 *   ├── triangle/
 *   │   └── triangle_1.png
 *   └── square/
 *       └── square_1.png
 */
void generateTrainingCorpus() {
    cout << "\n GENERANDO CORPUS DE ENTRENAMIENTO..." << endl;
    
    vector<ShapeDescriptor> corpus;
    vector<string> classes = {"circle", "triangle", "square"};
    
    for (const string& cls : classes) {
        string classDir = TRAIN_DIR + cls + "/";
        
        if (!filesystem::exists(classDir)) {
            cout << "  Directorio no existe: " << classDir << endl;
            continue;
        }
        
        for (const auto& entry : filesystem::directory_iterator(classDir)) {
            if (entry.path().extension() == ".png" || 
                entry.path().extension() == ".jpg") {
                
                Mat img = imread(entry.path().string());
                if (img.empty()) continue;
                
                ShapeDescriptor desc = extractShapeDescriptor(
                    img, cls, entry.path().filename().string()
                );
                
                if (!desc.features.empty()) {
                    corpus.push_back(desc);
                }
            }
        }
    }
    
    saveCorpus(corpus, "../data/corpus.csv");
    
    cout << "\n CORPUS GENERADO: " << corpus.size() << " ejemplos" << endl;
}

// FUNCIÓN PRINCIPAL: EVALUAR EN DATASET DE PRUEBA

void evaluateTestSet() {
    cout << "\n EVALUANDO DATASET DE PRUEBA..." << endl;
    
    // Cargar corpus
    auto corpus = loadCorpus("../data/corpus.csv");
    if (corpus.empty()) {
        cerr << " No se pudo cargar el corpus" << endl;
        return;
    }
    
    // Matriz de confusión
    map<string, map<string, int>> confusionMatrix;
    vector<string> classes = {"circle", "triangle", "square"};
    
    for (const string& cls : classes) {
        string classDir = TEST_DIR + cls + "/";
        
        if (!filesystem::exists(classDir)) {
            cout << "  Directorio no existe: " << classDir << endl;
            continue;
        }
        
        for (const auto& entry : filesystem::directory_iterator(classDir)) {
            if (entry.path().extension() == ".png" || 
                entry.path().extension() == ".jpg") {
                
                Mat img = imread(entry.path().string());
                if (img.empty()) continue;
                
                ShapeDescriptor desc = extractShapeDescriptor(
                    img, cls, entry.path().filename().string()
                );
                
                if (desc.features.empty()) continue;
                
                auto [predicted, distance] = classify(desc, corpus);
                
                confusionMatrix[cls][predicted]++;
                
                string status = (predicted == cls) ? "✓" : "✗";
                cout << status << " Real: " << cls << " | Predicho: " 
                     << predicted << " | Distancia: " << distance << endl;
            }
        }
    }
    
    // Imprimir matriz de confusión
    cout << "\n MATRIZ DE CONFUSIÓN:" << endl;
    cout << "           ";
    for (const auto& c : classes) cout << c << "\t";
    cout << endl;
    
    for (const auto& real : classes) {
        cout << real << "\t";
        for (const auto& pred : classes) {
            cout << confusionMatrix[real][pred] << "\t";
        }
        cout << endl;
    }
    
    // Calcular accuracy
    int total = 0, correct = 0;
    for (const auto& cls : classes) {
        correct += confusionMatrix[cls][cls];
        for (const auto& pred : classes) {
            total += confusionMatrix[cls][pred];
        }
    }
    
    float accuracy = (total > 0) ? (100.0f * correct / total) : 0.0f;
    cout << "\n ACCURACY: " << accuracy << "%" << endl;
}

// MAIN: MENÚ PRINCIPAL

int main(int argc, char** argv) {
    cout << "================================================" << endl;
    cout << "  SHAPE SIGNATURE - FFT COORDENADAS COMPLEJAS  " << endl;
    cout << "       Práctica 3-2 - Visión por Computador    " << endl;
    cout << "================================================" << endl;
    
    if (argc < 2) {
        cout << "\nUso:" << endl;
        cout << "  ./shape_app train         - Generar corpus de entrenamiento" << endl;
        cout << "  ./shape_app test          - Evaluar dataset de prueba" << endl;
        cout << "  ./shape_app classify <img> - Clasificar una imagen" << endl;
        return 0;
    }
    
    string mode = argv[1];
    
    if (mode == "train") {
        generateTrainingCorpus();
    } 
    else if (mode == "test") {
        evaluateTestSet();
    } 
    else if (mode == "classify" && argc >= 3) {
        string imgPath = argv[2];
        Mat img = imread(imgPath);
        
        if (img.empty()) {
            cerr << " No se pudo cargar imagen: " << imgPath << endl;
            return -1;
        }
        
        auto corpus = loadCorpus("../data/corpus.csv");
        auto desc = extractShapeDescriptor(img, "", imgPath);
        
        if (!desc.features.empty()) {
            auto [predicted, distance] = classify(desc, corpus);
            cout << "\n RESULTADO: " << predicted 
                 << " (distancia: " << distance << ")" << endl;
        }
    } 
    else {
        cerr << " Modo no reconocido: " << mode << endl;
        return -1;
    }
    
    return 0;
}