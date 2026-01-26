#include <jni.h>
#include <string>
#include <android/bitmap.h>
#include <android/log.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <cmath>
#include <numeric>

// Macros para logs
#define LOG_TAG "ShapeSignature"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace cv;
using namespace std;

const int NUM_POINTS = 1024;
const int NUM_HARMONICS = 15;

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
    float totalLength = cumulativeLength[n-1];

    vector<Point2f> interpolated(NUM_POINTS);
    for (int i = 0; i < NUM_POINTS; i++) {
        float targetLength = (totalLength * i) / NUM_POINTS;
        int idx = 0;
        while (idx < n-1 && cumulativeLength[idx+1] < targetLength) idx++;

        if (idx < n-1) {
            float segmentLength = cumulativeLength[idx+1] - cumulativeLength[idx];
            float t = (targetLength - cumulativeLength[idx]) / segmentLength;
            interpolated[i].x = (1-t) * contour[idx].x + t * contour[idx+1].x;
            interpolated[i].y = (1-t) * contour[idx].y + t * contour[idx+1].y;
        } else {
            interpolated[i] = contour[idx];
        }
    }
    return interpolated;
}

Point2f calculateCentroid(const vector<Point2f>& contour) {
    float sumX = 0, sumY = 0;
    for (const auto& pt : contour) { sumX += pt.x; sumY += pt.y; }
    return Point2f(sumX / contour.size(), sumY / contour.size());
}

void computeShapeSignature(const vector<Point>& rawContour, vector<float>& outDescriptor) {
    vector<Point2f> contour = interpolateContour(rawContour);
    if (contour.empty()) return;

    Point2f centroid = calculateCentroid(contour);

    Mat complexSignal(NUM_POINTS, 1, CV_32FC2);
    for (int i = 0; i < NUM_POINTS; i++) {
        complexSignal.at<Vec2f>(i, 0) = Vec2f(contour[i].x - centroid.x, contour[i].y - centroid.y);
    }

    Mat dftOutput;
    dft(complexSignal, dftOutput, DFT_COMPLEX_OUTPUT);

    vector<float> magnitudes;
    float f1 = 0;

    for (int i = 0; i < min(NUM_POINTS, NUM_HARMONICS + 2); i++) {
        Vec2f val = dftOutput.at<Vec2f>(i, 0);
        float mag = sqrt(val[0]*val[0] + val[1]*val[1]);

        if (i == 1) f1 = mag;
        if (i >= 1) magnitudes.push_back(mag);
    }

    outDescriptor.clear();
    if (f1 > 1e-5) {
        // Normalizamos dividiendo por F1.
        for (int k = 0; k < magnitudes.size() && k < NUM_HARMONICS; k++) {
            outDescriptor.push_back(magnitudes[k] / f1);
        }
    }
}

float euclideanDistance(const vector<float>& d1, const vector<float>& d2) {
    float sum = 0.0f;
    int size = min(d1.size(), d2.size());
    for (int i = 0; i < size; i++) {
        float diff = d1[i] - d2[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

// ============================================================================
// BLOQUE 2: CLASIFICACIÓN CALIBRADA (Con datos de tu CSV)
// ============================================================================

string classify(const vector<float>& signature) {
    if (signature.empty()) return "Ruido / Error";
    
    // Círculo: Valores bajos y suaves
    vector<float> refCircle = {
            1.00000f, 0.27562f, 0.25672f, 0.16020f, 0.09020f,
            0.05837f, 0.36992f, 0.03400f, 0.06516f, 0.04045f,
            0.02999f, 0.03970f, 0.02729f, 0.02794f, 0.05131f
    };

    // Cuadrado: Tiene armónicos altos muy marcados (2.16 en el tercer componente)
    vector<float> refSquare = {
            1.00000f, 1.08355f, 2.16746f, 0.66692f, 0.47570f,
            0.80803f, 0.60817f, 0.59543f, 0.49176f, 0.31772f,
            0.34150f, 0.29766f, 0.27031f, 0.21139f, 0.18143f
    };

    // Triángulo: Fuerte en el segundo componente (1.76)
    vector<float> refTriangle = {
            1.00000f, 1.76628f, 0.35032f, 0.18150f, 0.20200f,
            0.07263f, 0.05904f, 0.07821f, 0.05490f, 0.03887f,
            0.03076f, 0.02232f, 0.02314f, 0.01829f, 0.02023f
    };

    float dCircle = euclideanDistance(signature, refCircle);
    float dTriangle = euclideanDistance(signature, refTriangle);
    float dSquare = euclideanDistance(signature, refSquare);

    // LOG PARA DEPURAR (Mira esto en Logcat si sigue fallando)
    LOGI("Distancias -> C: %.3f, T: %.3f, S: %.3f", dCircle, dTriangle, dSquare);

    if (dCircle < dTriangle && dCircle < dSquare) return "Círculo";
    if (dTriangle < dCircle && dTriangle < dSquare) return "Triángulo";
    if (dSquare < dCircle && dSquare < dTriangle) return "Cuadrado";

    return "Desconocido";
}

// ============================================================================
// BLOQUE 3: PUENTE JNI
// ============================================================================

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_android_1app_MainActivity_classifyImage(
        JNIEnv* env, jobject, jobject bitmap) {

    AndroidBitmapInfo info;
    void* pixels;

    if (AndroidBitmap_getInfo(env, bitmap, &info) < 0 ||
        AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) {
        return env->NewStringUTF("Error JNI Bitmap");
    }

    Mat src(info.height, info.width, CV_8UC4, pixels);
    Mat gray, binary;

    cvtColor(src, gray, COLOR_RGBA2GRAY);
    // Usamos AdaptiveThreshold (igual que tu compañera)
    adaptiveThreshold(gray, binary, 255, ADAPTIVE_THRESH_GAUSSIAN_C,
                      THRESH_BINARY_INV, 11, 2);

    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
    morphologyEx(binary, binary, MORPH_CLOSE, kernel);

    vector<vector<Point>> contours;
    findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    string result = "Dibuja algo...";

    if (!contours.empty()) {
        int maxIdx = 0;
        double maxArea = 0;
        for (size_t i = 0; i < contours.size(); i++) {
            double area = contourArea(contours[i]);
            if (area > maxArea) {
                maxArea = area;
                maxIdx = i;
            }
        }

        if (maxArea > 500) {
            vector<float> signature;
            computeShapeSignature(contours[maxIdx], signature);
            result = classify(signature);
        } else {
            result = "Muy pequeño";
        }
    }

    AndroidBitmap_unlockPixels(env, bitmap);
    return env->NewStringUTF(result.c_str());
}