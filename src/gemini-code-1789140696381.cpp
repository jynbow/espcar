// Incluye OpenCV
#include <opencv2/opencv.hpp>
// Incluye consola
#include <iostream>
// Incluye vectores
#include <vector>
// Incluye algoritmos
#include <algorithm>
// Incluye limites numericos
#include <limits>
// Incluye biblioteca matematica
#include <cmath>
// Incluye sistema de archivos
#include <filesystem>

// Calcula histograma normalizado
std::vector<double> calcularHistogramaNormalizado(const cv::Mat& src) {
    // Vector de 256 ceros
    std::vector<double> hist(256, 0.0);
    // Total pixeles
    int total = src.rows * src.cols;
    // Recorre filas
    for (int r = 0; r < src.rows; ++r) {
        // Puntero contiguo
        const uchar* ptr = src.ptr<uchar>(r);
        // Recorre columnas
        for (int c = 0; c < src.cols; ++c) hist[ptr[c]] += 1.0;
    }
    // Normaliza
    for (int i = 0; i < 256; ++i) hist[i] /= static_cast<double>(total);
    // Retorna histograma
    return hist;
}

// Calcula CDF
std::vector<double> calcularCDF(const std::vector<double>& hist) {
    // Vector CDF
    std::vector<double> cdf(256, 0.0);
    // Acumulador
    double acc = 0.0;
    // Itera por niveles
    for (int i = 0; i < 256; ++i) {
        // Acumula
        acc += hist[i];
        // Asigna
        cdf[i] = acc;
    }
    // Retorna CDF
    return cdf;
}

// Genera tabla LUT de matching
cv::Mat crearLUTMatching(const cv::Mat& src, const std::vector<double>& cdf_ref) {
    // Histogramas y CDF
    std::vector<double> hist_src = calcularHistogramaNormalizado(src);
    std::vector<double> cdf_src = calcularCDF(hist_src);
    // Matriz de mapeo
    cv::Mat lut(1, 256, CV_8U);
    uchar* ptr = lut.ptr();

    // Recorre niveles de entrada
    for (int r = 0; r < 256; ++r) {
        // Acumulado de entrada
        double val = cdf_src[r];
        double min_diff = std::numeric_limits<double>::max();
        uchar mejor_s = 0;
        // Busca en referencia
        for (int s = 0; s < 256; ++s) {
            // Discrepancia absoluta
            double diff = std::abs(val - cdf_ref[s]);
            // Actualiza coincidencia
            if (diff < min_diff) {
                // Asigna distancia
                min_diff = diff;
                // Guarda nivel
                mejor_s = static_cast<uchar>(s);
            }
        }
        // Asigna traduccion
        ptr[r] = mejor_s;
    }
    // Retorna LUT
    return lut;
}

// Estructura de evaluacion de linea
struct CuadroInfo {
    // Indice cronologico
    int indice;
    // Pixeles segmentados
    int pixeles_linea;
};

// Punto de entrada principal
int main() {
    // Directorio raiz
    std::string carpeta_raiz = "C:/Users/Lenovo/Videos/control/";
    // Carpeta destino
    std::string carpeta_dst = carpeta_raiz + "punto5histograma/";
    // Crea carpeta en disco
    std::filesystem::create_directories(carpeta_dst);

    // Archivo fuente
    std::string path_in = carpeta_raiz + "ideal.mp4";
    // Abre captura
    cv::VideoCapture cap(path_in);
    // Valida lectura
    if (!cap.isOpened()) return -1;

    // Metadatos
    double fps = cap.get(cv::CAP_PROP_FPS);
    // Ancho
    int ancho = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    // Alto
    int alto = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    // Cuadros para 15s
    int total_frames = static_cast<int>(fps * 15.0);

    // Almacena fotogramas
    std::vector<cv::Mat> cuadros;
    cuadros.reserve(total_frames);
    // Carga fotogramas
    cv::Mat frame_bgr, frame_gray;
    for (int i = 0; i < total_frames; ++i) {
        // Lee fotograma
        if (!cap.read(frame_bgr)) break;
        // Convierte a grises
        cv::cvtColor(frame_bgr, frame_gray, cv::COLOR_BGR2GRAY);
        // Agrega al vector
        cuadros.push_back(frame_gray.clone());
    }
    // Libera decodificador
    cap.release();

    // Valida cantidad de fotogramas
    if (cuadros.size() < 10) return -1;

    // Codec de video
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');

    // 1. Umbral base mediante Otsu
    cv::Mat dummy;
    // Calcula umbral
    double umbral_base = cv::threshold(cuadros[9], dummy, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

    // Genera 10 umbrales de prueba
    std::vector<int> variaciones_umbral;
    for (int k = 0; k < 10; ++k) {
        // Desplaza el corte
        int u = static_cast<int>(umbral_base) - 25 + (k * 5);
        // Satura limites
        variaciones_umbral.push_back(std::clamp(u, 1, 254));
    }

    // Exporta los 10 videos de binarizacion global (5.a)
    for (int idx = 0; idx < 10; ++idx) {
        // Umbral actual
        int T = variaciones_umbral[idx];
        // Prefijo numerico
        std::string prefijo = (idx < 9) ? "exp0" : "exp";
        // Nombre de archivo
        std::string path_out = carpeta_dst + prefijo + std::to_string(idx + 1) + "_p5a_umbral_" + std::to_string(T) + ".mp4";
        // Grabador monocromatico
        cv::VideoWriter writer(path_out, fourcc, fps, cv::Size(ancho, alto), false);
        // Valida grabador
        if (!writer.isOpened()) continue;

        // Binariza fotogramas
        cv::Mat frame_bw;
        for (const auto& c : cuadros) {
            // Aplica corte
            cv::threshold(c, frame_bw, T, 255, cv::THRESH_BINARY);
            // Graba fotograma
            writer.write(frame_bw);
        }
        // Cierra MP4 de forma segura
        writer.release();
    }

    // 2. Ranking de cuadros con menor linea
    std::vector<CuadroInfo> ranking;
    for (size_t i = 0; i < cuadros.size(); ++i) {
        // Mascara binaria
        cv::Mat mascara;
        // Segmenta linea
        cv::threshold(cuadros[i], mascara, umbral_base, 255, cv::THRESH_BINARY_INV);
        // Agrega al ranking
        ranking.push_back({static_cast<int>(i), cv::countNonZero(mascara)});
    }
    // Ordena de menor a mayor cantidad de linea
    std::sort(ranking.begin(), ranking.end(), [](const CuadroInfo& a, const CuadroInfo& b) {
        return a.pixeles_linea < b.pixeles_linea;
    });

    // 3. Exporta 10 videos de Histogram Matching (5.c)
    for (int idx = 0; idx < 10; ++idx) {
        // Cuadro patron
        int ref = ranking[idx].indice;
        // CDF de referencia
        std::vector<double> cdf_ref = calcularCDF(calcularHistogramaNormalizado(cuadros[ref]));
        // Prefijo numerico
        std::string prefijo = (idx < 9) ? "exp0" : "exp";
        // Nombre de salida
        std::string path_out = carpeta_dst + prefijo + std::to_string(idx + 1) + "_p5c_matching_frame_" + std::to_string(ref) + ".mp4";
        // Grabador monocromatico
        cv::VideoWriter writer(path_out, fourcc, fps, cv::Size(ancho, alto), false);
        // Valida grabador
        if (!writer.isOpened()) continue;

        // Transfiere contraste cuadro a cuadro
        cv::Mat frame_matched;
        for (size_t f = 0; f < cuadros.size(); ++f) {
            // Evalua si es el cuadro patron
            if (static_cast<int>(f) == ref) {
                // Escribe cuadro original
                writer.write(cuadros[f]);
            } else {
                // Genera LUT
                cv::Mat lut = crearLUTMatching(cuadros[f], cdf_ref);
                // Aplica transformacion
                cv::LUT(cuadros[f], lut, frame_matched);
                // Escribe fotograma
                writer.write(frame_matched);
            }
        }
        // Cierra MP4 de forma segura
        writer.release();
    }

    // Retorna exito
    return 0;
}