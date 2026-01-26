#include <jni.h>
#include <string>
#include <android/bitmap.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <sstream>
#include <cmath>

using namespace cv;
using namespace std;

#define LOG_TAG "FFT_Classifier"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


// CONSTANTES GLOBALES


const int NUM_POINTS = 1024;
const int NUM_HARMONICS = 15;

<<<<<<< HEAD
// ============================================================================
// BLOQUE 1: LÓGICA MATEMÁTICA (Shape Signature)
// ============================================================================

vector<Point2f> interpolateContour(const vector<Point>& contour) {
    int n = contour.size();
    if (n < 3) return vector<Point2f>();

    vector<float> cumulativeLength(n);
    cumulativeLength[0] = 0.0f;
    for (int i = 1; i < n; i++) {
        float dx = contour[i].x - contour[i-1].x;
        float dy = contour[i].y - contour[i-1].y;
        cumulativeLength[i] = cumulativeLength[i-1] + sqrt(dx*dx + dy*dy);
    }
    
    if (image.channels() == 3 || image.channels() == 4) {
        cvtColor(image, gray, COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    adaptiveThreshold(gray, binary, 255, ADAPTIVE_THRESH_GAUSSIAN_C, 
                      THRESH_BINARY_INV, 11, 2);
    
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
    morphologyEx(binary, binary, MORPH_CLOSE, kernel);
    morphologyEx(binary, binary, MORPH_OPEN, kernel);
    
    vector<vector<Point>> contours;
    findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    
    if (contours.empty()) {
        LOGE("No se encontraron contornos");
        return false;
    }
    
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
    
    if (maxArea < 100) {
        LOGE("Contorno muy pequeño (área < 100)");
        return false;
    }
    
    LOGI("Contorno extraído: %zu puntos, área = %.0f px²", contour.size(), maxArea);
    return true;
}

// PASO 2: INTERPOLACIÓN LINEAL A 1024 PUNTOS

vector<Point2f> interpolateContour(const vector<Point>& contour) {
    int n = contour.size();
    
    if (n < 3) {
        LOGE("Contorno con muy pocos puntos: %d", n);
        return vector<Point2f>();
    }
    
    vector<float> cumulativeLength(n);
    cumulativeLength[0] = 0.0f;
    
    for (int i = 1; i < n; i++) {
        float dx = contour[i].x - contour[i-1].x;
        float dy = contour[i].y - contour[i-1].y;
        float dist = sqrt(dx*dx + dy*dy);
        cumulativeLength[i] = cumulativeLength[i-1] + dist;
    }
    
    float totalLength = cumulativeLength[n-1];
    vector<Point2f> interpolated(NUM_POINTS);
    
    for (int i = 0; i < NUM_POINTS; i++) {
        float targetLength = (totalLength * i) / NUM_POINTS;
        
        int idx = 0;
        while (idx < n-1 && cumulativeLength[idx+1] < targetLength) {
            idx++;
        }
        
        if (idx < n-1) {
            float segmentLength = cumulativeLength[idx+1] - cumulativeLength[idx];
            float t = (targetLength - cumulativeLength[idx]) / segmentLength;
            
>>>>>>> c89b9f39f967c6e7ce3c1945fc2c7bb9c3374d56
            interpolated[i].x = (1-t) * contour[idx].x + t * contour[idx+1].x;
            interpolated[i].y = (1-t) * contour[idx].y + t * contour[idx+1].y;
        } else {
            interpolated[i] = contour[idx];
        }
    }
    
    LOGI("Contorno interpolado: %d → %d puntos", n, NUM_POINTS);
    return interpolated;
}

// PASO 3: CALCULAR CENTROIDE

Point2f calculateCentroid(const vector<Point2f>& contour) {
    float sumX = 0, sumY = 0;
    
    for (const auto& pt : contour) {
        sumX += pt.x;
        sumY += pt.y;
    }
    
    Point2f centroid(sumX / contour.size(), sumY / contour.size());
    LOGI("Centroide: (%.2f, %.2f)", centroid.x, centroid.y);
    
    return centroid;
}

// PASO 4: CONSTRUIR SEÑAL COMPLEJA

Mat buildComplexSignal(const vector<Point2f>& contour, const Point2f& centroid) {
    int n = contour.size();
    Mat complexSignal(n, 1, CV_32FC2);
    
    for (int i = 0; i < n; i++) {
        float real = contour[i].x - centroid.x;
        float imag = contour[i].y - centroid.y;
        complexSignal.at<Vec2f>(i, 0) = Vec2f(real, imag);
    }
    
    LOGI("Señal compleja construida");
    return complexSignal;
}

// PASO 5: FFT

void computeFFT(const Mat& complexSignal, vector<float>& magnitudes) {
    Mat dftOutput;
    dft(complexSignal, dftOutput, DFT_COMPLEX_OUTPUT);
    
    vector<Mat> planes(2);
    split(dftOutput, planes);
    
    Mat mag;
    magnitude(planes[0], planes[1], mag);
    
    magnitudes.clear();
    for (int i = 0; i < mag.rows; i++) {
        magnitudes.push_back(mag.at<float>(i, 0));
    }
    
    LOGI("FFT calculada: %zu coeficientes", magnitudes.size());
}

// PASO 6: NORMALIZACIÓN

vector<float> normalizeDescriptor(const vector<float>& magnitudes) {
    if (magnitudes.size() < 2) {
        LOGE("Muy pocos coeficientes de Fourier");
        return vector<float>(NUM_HARMONICS, 0.0f);
    }
    
    float fundamental = magnitudes[1];
    
    if (fundamental < 1e-5) {
        LOGE("Fundamental muy pequeño");
        return vector<float>(NUM_HARMONICS, 0.0f);
    }
    
    vector<float> descriptor;
    
    for (int k = 1; k <= NUM_HARMONICS && k < magnitudes.size(); k++) {
        float normalized = magnitudes[k] / fundamental;
        descriptor.push_back(normalized);
    }
    
    while (descriptor.size() < NUM_HARMONICS) {
        descriptor.push_back(0.0f);
    }
    
    LOGI("Descriptor normalizado: %zu armónicos", descriptor.size());
    return descriptor;
}

// PIPELINE COMPLETO: EXTRAER DESCRIPTOR

ShapeDescriptor extractShapeDescriptor(const Mat& image) {
    LOGI("========================================");
    LOGI("Iniciando extracción de descriptor");
    LOGI("========================================");
    
    vector<Point> contour;
    if (!extractContour(image, contour)) {
        return ShapeDescriptor();
    }
    
    vector<Point2f> interpolated = interpolateContour(contour);
    if (interpolated.empty()) {
        return ShapeDescriptor();
    }
    
    Point2f centroid = calculateCentroid(interpolated);
    Mat complexSignal = buildComplexSignal(interpolated, centroid);
    
    vector<float> magnitudes;
    computeFFT(complexSignal, magnitudes);
    
    vector<float> descriptor = normalizeDescriptor(magnitudes);
    
    LOGI("Descriptor extraído exitosamente");
    return ShapeDescriptor(descriptor, "");
}

// CLASIFICACIÓN: DISTANCIA EUCLÍDEA

float euclideanDistance(const vector<float>& d1, const vector<float>& d2) {
    if (d1.size() != d2.size()) {
        LOGE("Descriptores de diferente tamaño");
        return 1e9;
    }
    
    float sum = 0.0f;
    for (size_t i = 0; i < d1.size(); i++) {
        float diff = d1[i] - d2[i];
        sum += diff * diff;
    }
    
    return sqrt(sum);
}

pair<string, float> classify(const ShapeDescriptor& testDescriptor, 
                             const vector<ShapeDescriptor>& corpus) {
    if (corpus.empty()) {
        LOGE("Corpus vacío");
        return {"unknown", 1e9};
    }
    
    string bestLabel = "unknown";
    float minDistance = 1e9;
    
    for (const auto& train : corpus) {
        float dist = euclideanDistance(testDescriptor.features, train.features);
        
        if (dist < minDistance) {
            minDistance = dist;
            bestLabel = train.label;
        }
    }
    
    LOGI("Clasificación: %s (distancia: %.4f)", bestLabel.c_str(), minDistance);
    return {bestLabel, minDistance};
}

// CARGAR CORPUS DESDE ASSETS

vector<ShapeDescriptor> loadCorpusFromAssets(AAssetManager* assetManager) {
    vector<ShapeDescriptor> corpus;
    
    AAsset* asset = AAssetManager_open(assetManager, "corpus.csv", AASSET_MODE_BUFFER);
    if (!asset) {
        LOGE("No se pudo abrir corpus.csv desde assets");
        return corpus;
    }
    
    size_t fileSize = AAsset_getLength(asset);
    const char* buffer = static_cast<const char*>(AAsset_getBuffer(asset));
    
    stringstream ss(string(buffer, fileSize));
    string line;
    
    while (getline(ss, line)) {
        stringstream lineStream(line);
        string label;
        getline(lineStream, label, ',');
        
        vector<float> features;
        string value;
        while (getline(lineStream, value, ',')) {
            features.push_back(stof(value));
        }
        
        corpus.push_back(ShapeDescriptor(features, label));
    }
    
    AAsset_close(asset);
    LOGI("Corpus cargado: %zu ejemplos", corpus.size());
    
    return corpus;
}

// CONVERSIÓN: Android Bitmap → OpenCV Mat

Mat bitmapToMat(JNIEnv* env, jobject bitmap) {
    AndroidBitmapInfo info;
    void* pixels;
    
    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    
    Mat mat(info.height, info.width, CV_8UC4, pixels);
    Mat result;
    mat.copyTo(result);
    
    AndroidBitmap_unlockPixels(env, bitmap);
    
    return result;
}

// TRADUCCIÓN A ESPAÑOL

string translateToSpanish(const string& label) {
    if (label == "circle") return "Círculo";
    if (label == "triangle") return "Triángulo";
    if (label == "square") return "Cuadrado";
    return label;
}

// JNI: FUNCIÓN DE CLASIFICACIÓN

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_android_1app_MainActivity_classifyImage(
        JNIEnv* env,
        jobject /* this */,
        jobject bitmap,
        jobject assetManager) {
    
    LOGI("========================================");
    LOGI("JNI: Iniciando clasificación");
    LOGI("========================================");
    
    // Convertir Bitmap a Mat
    Mat image = bitmapToMat(env, bitmap);
    LOGI("Imagen recibida: %dx%d", image.cols, image.rows);
    
    // Cargar corpus
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    vector<ShapeDescriptor> corpus = loadCorpusFromAssets(mgr);
    
    if (corpus.empty()) {
        return env->NewStringUTF("Error: Corpus vacío");
    }
    
    // Extraer descriptor de la imagen
    ShapeDescriptor testDescriptor = extractShapeDescriptor(image);
    
    if (testDescriptor.features.empty()) {
        return env->NewStringUTF("Error: No se pudo extraer descriptor");
    }
    
    // Clasificar
    auto [label, distance] = classify(testDescriptor, corpus);
    
    // Traducir a español
    string result = translateToSpanish(label);
    LOGI("Resultado final: %s", result.c_str());
    
    return env->NewStringUTF(result.c_str());
}