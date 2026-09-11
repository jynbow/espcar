// Incluye OpenCV
#include <opencv2/opencv.hpp>
// Incluye consola
#include <iostream>
// Incluye vectores
#include <vector>
// Incluye biblioteca matematica
#include <cmath>

// Genera tabla LUT para gamma
cv::Mat crearTablaGamma(double gamma) {
    // Matriz de 256 niveles
    cv::Mat lut(1, 256, CV_8U);
    uchar* ptr = lut.ptr();
    // Itera por niveles
    for (int i = 0; i < 256; ++i) {
        // Aplica transformacion potencial
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

            // Busca el vecino mas afan
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

// Superpone un rotulo de texto superior
void agregarEtiqueta(cv::Mat& img, const std::string& texto) {
    // Barra superior
    cv::rectangle(img, cv::Point(0, 0), cv::Point(img.cols, 24), cv::Scalar(0), cv::FILLED);
    // Texto
    cv::putText(img, texto, cv::Point(8, 16), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(255), 1, cv::LINE_AA);
}

// Punto de entrada principal
int main() {
    // Carpeta base
    std::string carpeta_base = "C:/Users/Lenovo/Videos/control/";
    // Video a inspeccionar
    std::string archivo_video = "oscuro.mp4";
    // Gamma
    double gamma_val = (archivo_video == "oscuro.mp4") ? 0.40 : 1.50;

    // Abre captura
    cv::VideoCapture cap(carpeta_base + archivo_video);
    if (!cap.isOpened()) return -1;

    // Metadatos
    double fps = cap.get(cv::CAP_PROP_FPS);
    int total_frames = static_cast<int>(fps * 15.0);

    // Precalcula LUT
    cv::Mat lut = crearTablaGamma(gamma_val);

    // Ventana mosaico
    std::string ventana = "Comparador Multivista: " + archivo_video;
    cv::namedWindow(ventana, cv::WINDOW_AUTOSIZE);

    // Matrices intermedias
    cv::Mat frame_bgr, frame_gray, frame_gamma, frame_prom, frame_med, frame_vec;

    // Itera por los cuadros
    for (int f = 0; f < total_frames; ++f) {
        if (!cap.read(frame_bgr)) break;

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

        // Copias para rotular
        cv::Mat v1 = frame_gray.clone();
        cv::Mat v2 = frame_gamma.clone();
        cv::Mat v3 = frame_prom.clone();
        cv::Mat v4 = frame_med.clone();
        cv::Mat v5 = frame_vec.clone();

        // Agrega etiquetas
        agregarEtiqueta(v1, "1. Original (Grises)");
        agregarEtiqueta(v2, "2. Solo Gamma (" + std::to_string(gamma_val).substr(0, 4) + ")");
        agregarEtiqueta(v3, "3. Promedio (3x3)");
        agregarEtiqueta(v4, "4. Mediana (3x3)");
        agregarEtiqueta(v5, "5. Mejor Vecino (3x3)");

        // Panel informativo
        cv::Mat v6 = cv::Mat::zeros(frame_gray.size(), CV_8U);
        agregarEtiqueta(v6, "Panel Informativo");
        cv::putText(v6, "Cuadro #" + std::to_string(f) + "/" + std::to_string(total_frames), 
                    cv::Point(20, 80), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255), 1, cv::LINE_AA);
        cv::putText(v6, "Video: " + archivo_video, 
                    cv::Point(20, 110), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255), 1, cv::LINE_AA);
        cv::putText(v6, "Gamma: " + std::to_string(gamma_val).substr(0, 4), 
                    cv::Point(20, 140), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255), 1, cv::LINE_AA);
        cv::putText(v6, "Presione ESC para salir", 
                    cv::Point(20, 190), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(180), 1, cv::LINE_AA);

        // Concatena filas
        cv::Mat f_sup, f_inf, mosaico;
        cv::hconcat(std::vector<cv::Mat>{v1, v2, v3}, f_sup);
        cv::hconcat(std::vector<cv::Mat>{v4, v5, v6}, f_inf);
        cv::vconcat(f_sup, f_inf, mosaico);

        // Despliega mosaico
        cv::imshow(ventana, mosaico);

        // Temporizador
        char c = static_cast<char>(cv::waitKey(static_cast<int>(1000.0 / fps)));
        if (c == 27 || c == 'q') break;
    }

    // Libera recursos
    cap.release();
    cv::destroyAllWindows();

    // Retorna exito
    return 0;
}