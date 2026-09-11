// Incluye OpenCV
#include <opencv2/opencv.hpp>
// Incluye consola
#include <iostream>
// Incluye vectores
#include <vector>
// Incluye biblioteca matematica
#include <cmath>
// Incluye cadenas
#include <sstream>
// Incluye manipuladores
#include <iomanip>
// Incluye sistema de archivos
#include <filesystem>

// Genera tabla LUT para gamma
cv::Mat crearTablaGamma(double gamma) {
    // Matriz de 256 niveles
    cv::Mat lut(1, 256, CV_8U);
    // Puntero contiguo
    uchar* ptr = lut.ptr();
    // Itera por niveles
    for (int i = 0; i < 256; ++i) {
        // Aplica curva potencial
        ptr[i] = cv::saturate_cast<uchar>(std::pow(i / 255.0, gamma) * 255.0);
    }
    // Retorna tabla
    return lut;
}

// Algoritmo espacial del Mejor Vecino
cv::Mat aplicarMejorVecino(const cv::Mat& src) {
    // Clona matriz base
    cv::Mat dst = src.clone();
    // Filas
    int filas = src.rows;
    // Columnas
    int cols = src.cols;

    // Recorre filas internas
    for (int y = 1; y < filas - 1; ++y) {
        // Fila previa
        const uchar* prev = src.ptr<uchar>(y - 1);
        // Fila actual
        const uchar* curr = src.ptr<uchar>(y);
        // Fila siguiente
        const uchar* next = src.ptr<uchar>(y + 1);
        // Fila de salida
        uchar* out = dst.ptr<uchar>(y);

        // Recorre columnas internas
        for (int x = 1; x < cols - 1; ++x) {
            // Intensidad central
            uchar centro = curr[x];
            // Vecinos
            uchar vecinos[8] = {
                prev[x - 1], prev[x], prev[x + 1],
                curr[x - 1],           curr[x + 1],
                next[x - 1], next[x], next[x + 1]
            };

            // Menor discrepancia
            int min_diff = 1000;
            // Candidato
            uchar mejor = centro;

            // Busca el vecino mas afan
            for (int k = 0; k < 8; ++k) {
                // Modulo de la desviacion
                int diff = std::abs(static_cast<int>(centro) - static_cast<int>(vecinos[k]));
                // Actualiza minima
                if (diff < min_diff) {
                    // Asigna distancia
                    min_diff = diff;
                    // Asigna vecino
                    mejor = vecinos[k];
                }
            }
            // Guarda en destino
            out[x] = mejor;
        }
    }
    // Retorna matriz filtrada
    return dst;
}

// Estructura de caso de prueba
struct ExperimentoFiltro {
    // Tipo de filtro
    std::string tipo;
    // Correccion gamma
    double gamma;
    // Apertura del kernel
    int kernel_size;
};

// Punto de entrada principal
int main() {
    // Carpeta raiz
    std::string carpeta_raiz = "C:/Users/Lenovo/Videos/control/";
    // Videos base
    std::vector<std::string> lista_videos = {"oscuro", "ideal"};

    // Arreglo de 24 experimentos simetricos
    std::vector<ExperimentoFiltro> experimentos = {
        // Bloque 1: Promedio (10 casos)
        {"promedio",     0.30, 3}, {"promedio",     0.30, 5},
        {"promedio",     0.45, 3}, {"promedio",     0.45, 5}, {"promedio", 0.45, 7},
        {"promedio",     0.60, 3}, {"promedio",     0.60, 7},
        {"promedio",     0.80, 5},
        {"promedio",     1.00, 3},
        {"promedio",     1.30, 5},
        // Bloque 2: Mediana (10 casos)
        {"mediana",      0.25, 3},
        {"mediana",      0.35, 3}, {"mediana",      0.35, 5},
        {"mediana",      0.50, 3}, {"mediana",      0.50, 5}, {"mediana", 0.50, 7},
        {"mediana",      0.70, 5}, {"mediana",      0.70, 9},
        {"mediana",      1.00, 3},
        {"mediana",      1.50, 5},
        // Bloque 3: Mejor Vecino (4 casos)
        {"mejor_vecino", 0.30, 3},
        {"mejor_vecino", 0.45, 3},
        {"mejor_vecino", 0.60, 3},
        {"mejor_vecino", 1.00, 3}
    };

    // Codec de video
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');

    // Itera por los dos videos
    for (const auto& video : lista_videos) {
        // Asigna subcarpeta destino
        std::string carpeta_dst = carpeta_raiz + (video == "oscuro" ? "oscurofiltros/" : "idealfiltros/");
        // Crea carpeta en disco
        std::filesystem::create_directories(carpeta_dst);
        // Ruta de entrada
        std::string path_in = carpeta_raiz + video + ".mp4";

        // Itera por los 24 experimentos
        for (size_t idx = 0; idx < experimentos.size(); ++idx) {
            // Configuracion actual
            ExperimentoFiltro exp = experimentos[idx];
            // Precalcula LUT
            cv::Mat lut = crearTablaGamma(exp.gamma);

            // Abre decodificador
            cv::VideoCapture cap(path_in);
            // Valida apertura
            if (!cap.isOpened()) break;

            // Metadatos
            double fps = cap.get(cv::CAP_PROP_FPS);
            // Ancho
            int ancho = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
            // Alto
            int alto = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
            // Cuadros para 15s
            int total_frames = static_cast<int>(fps * 15.0);

            // Formatea gamma
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << exp.gamma;
            // Prefijo numerico ordenado
            std::string prefijo = (idx < 9) ? "exp0" : "exp";
            // Nombre de archivo
            std::string nombre_out = prefijo + std::to_string(idx + 1) + "_" + video + "_" + exp.tipo + "_k" + std::to_string(exp.kernel_size) + "_gamma_" + ss.str() + ".mp4";
            // Ruta final
            std::string path_out = carpeta_dst + nombre_out;

            // Instancia grabador
            cv::VideoWriter writer(path_out, fourcc, fps, cv::Size(ancho, alto), false);
            // Valida grabador
            if (!writer.isOpened()) { cap.release(); continue; }

            // Informa por consola
            std::cout << "-> [" << video << "] Prueba #" << (idx + 1) << " (" << exp.tipo << ")..." << std::flush;

            // Matrices de procesamiento
            cv::Mat frame_bgr, frame_gray, frame_gamma, frame_filtrado;
            // Procesa 15s
            for (int f = 0; f < total_frames; ++f) {
                // Lee fotograma
                if (!cap.read(frame_bgr)) break;
                // Convierte a grises
                cv::cvtColor(frame_bgr, frame_gray, cv::COLOR_BGR2GRAY);
                // Aplica gamma
                cv::LUT(frame_gray, lut, frame_gamma);

                // Aplica filtro
                if (exp.tipo == "promedio") {
                    // Filtro promedio
                    cv::blur(frame_gamma, frame_filtrado, cv::Size(exp.kernel_size, exp.kernel_size));
                } else if (exp.tipo == "mediana") {
                    // Filtro mediana
                    cv::medianBlur(frame_gamma, frame_filtrado, exp.kernel_size);
                } else if (exp.tipo == "mejor_vecino") {
                    // Mejor vecino
                    frame_filtrado = aplicarMejorVecino(frame_gamma);
                }
                // Graba fotograma
                writer.write(frame_filtrado);
            }
            // Libera captura
            cap.release();
            // Cierra MP4 de forma segura
            writer.release();
            // Confirma fin
            std::cout << " Listo." << std::endl;
        }
    }
    // Retorna exito
    return 0;
}