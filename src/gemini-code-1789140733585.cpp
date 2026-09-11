// Incluye OpenCV
#include <opencv2/opencv.hpp>
// Incluye consola
#include <iostream>
// Incluye vectores
#include <vector>
// Incluye biblioteca matematica
#include <cmath>
// Incluye manipuladores
#include <iomanip>

// Genera tabla LUT para gamma
cv::Mat crearTablaGamma(double gamma) {
    // Matriz de 256 niveles
    cv::Mat lut(1, 256, CV_8U);
    uchar* ptr = lut.ptr();
    // Itera por niveles
    for (int i = 0; i < 256; ++i) {
        // Aplica curva potencial
        ptr[i] = cv::saturate_cast<uchar>(std::pow(i / 255.0, gamma) * 255.0);
    }
    // Retorna LUT
    return lut;
}

// Algoritmo espacial del Mejor Vecino
cv::Mat aplicarMejorVecino(const cv::Mat& src) {
    // Clona matriz base
    cv::Mat dst = src.clone();
    int filas = src.rows;
    int cols = src.cols;

    // Recorre filas internas
    for (int y = 1; y < filas - 1; ++y) {
        const uchar* prev = src.ptr<uchar>(y - 1);
        const uchar* curr = src.ptr<uchar>(y);
        const uchar* next = src.ptr<uchar>(y + 1);
        uchar* out = dst.ptr<uchar>(y);

        // Recorre columnas internas
        for (int x = 1; x < cols - 1; ++x) {
            uchar centro = curr[x];
            uchar vecinos[8] = {
                prev[x - 1], prev[x], prev[x + 1],
                curr[x - 1],           curr[x + 1],
                next[x - 1], next[x], next[x + 1]
            };

            int min_diff = 1000;
            uchar mejor = centro;

            // Busca el vecino mas cercano en tono
            for (int k = 0; k < 8; ++k) {
                int diff = std::abs(static_cast<int>(centro) - static_cast<int>(vecinos[k]));
                if (diff < min_diff) {
                    min_diff = diff;
                    mejor = vecinos[k];
                }
            }
            out[x] = mejor;
        }
    }
    // Retorna matriz filtrada
    return dst;
}

// Calcula el SNR real del sensor en una region homogenea de fondo
double calcularSNRRealFondo(const cv::Mat& img) {
    // Region de fondo plana del suelo
    cv::Rect roi(img.cols * 0.55, img.rows * 0.40, img.cols * 0.25, img.rows * 0.25);
    cv::Mat fondo = img(roi);

    // Variables de media y desviacion estandar
    cv::Scalar media, stddev;
    cv::meanStdDev(fondo, media, stddev);

    // Varianza del ruido
    double potencia_ruido = stddev[0] * stddev[0];
    // Potencia de la senal
    double potencia_senal = media[0] * media[0];

    // Evita division por cero
    if (potencia_ruido < 1e-10) return 99.99;

    // Retorna SNR en dB
    return 10.0 * std::log10(potencia_senal / potencia_ruido);
}

// Estructura de resultados
struct MetricasSNR {
    std::string video;
    int cuadro;
    double snr_original;
    double snr_gamma;
    double snr_promedio;
    double snr_mediana;
    double snr_vecino;
};

// Punto de entrada principal
int main() {
    // Carpeta de videos
    std::string carpeta_base = "C:/Users/Lenovo/Videos/control/";
    // Videos base
    std::vector<std::string> lista_videos = {"ideal", "oscuro"};
    // Cuadros a evaluar
    std::vector<int> cuadros_eval = {5, 30, 90, 150};
    // Tabla general
    std::vector<MetricasSNR> tabla;

    // Recorre ambos videos
    for (const auto& video : lista_videos) {
        // Ruta de entrada
        std::string path_in = carpeta_base + video + ".mp4";
        // Abre decodificador
        cv::VideoCapture cap(path_in);
        if (!cap.isOpened()) continue;

        // Metadatos
        double fps = cap.get(cv::CAP_PROP_FPS);
        int total_frames = static_cast<int>(fps * 15.0);

        // Gamma de prueba
        double gamma_val = (video == "ideal") ? 1.50 : 0.40;
        // Precalcula LUT
        cv::Mat lut = crearTablaGamma(gamma_val);

        // Matrices intermedias
        cv::Mat frame_bgr, frame_gray, frame_gamma, frame_prom, frame_med, frame_vec;

        // Itera por los cuadros de 15 segundos
        for (int f = 0; f < total_frames; ++f) {
            if (!cap.read(frame_bgr)) break;

            // Evalua si coincide con el cuadro de prueba
            for (int q : cuadros_eval) {
                if (f == q) {
                    // Pasa a grises
                    cv::cvtColor(frame_bgr, frame_gray, cv::COLOR_BGR2GRAY);
                    // Aplica gamma
                    cv::LUT(frame_gray, lut, frame_gamma);
                    // Aplica promedio
                    cv::blur(frame_gamma, frame_prom, cv::Size(3, 3));
                    // Aplica mediana
                    cv::medianBlur(frame_gamma, frame_med, 3);
                    // Aplica mejor vecino
                    frame_vec = aplicarMejorVecino(frame_gamma);

                    // Agrega fila a la tabla
                    tabla.push_back({
                        video, f,
                        calcularSNRRealFondo(frame_gray),
                        calcularSNRRealFondo(frame_gamma),
                        calcularSNRRealFondo(frame_prom),
                        calcularSNRRealFondo(frame_med),
                        calcularSNRRealFondo(frame_vec)
                    });
                }
            }
        }
        // Libera decodificador
        cap.release();
    }

    // Despliega tabla en consola
    std::cout << "\n============================================================================================================" << std::endl;
    std::cout << "                 TABLA DE SNR REAL (db) RESPECTO AL RUIDO DEL SENSOR (FONDO HOMOGENEO)                      " << std::endl;
    std::cout << "============================================================================================================" << std::endl;
    std::cout << std::setw(8)  << "Video"
              << std::setw(10) << "Cuadro #"
              << std::setw(18) << "SNR Original"
              << std::setw(16) << "SNR Gamma"
              << std::setw(18) << "SNR Promedio"
              << std::setw(17) << "SNR Mediana"
              << std::setw(18) << "SNR Vecino" << std::endl;
    std::cout << "------------------------------------------------------------------------------------------------------------" << std::endl;

    for (const auto& r : tabla) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << std::setw(8)  << r.video
                  << std::setw(10) << r.cuadro
                  << std::setw(18) << r.snr_original
                  << std::setw(16) << r.snr_gamma
                  << std::setw(18) << r.snr_promedio
                  << std::setw(17) << r.snr_mediana
                  << std::setw(18) << r.snr_vecino << std::endl;
    }
    std::cout << "============================================================================================================" << std::endl;

    // Retorna exito
    return 0;
}