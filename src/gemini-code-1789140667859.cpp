// Incluye OpenCV
#include <opencv2/opencv.hpp>
// Incluye consola estandar
#include <iostream>
// Incluye cadenas de texto
#include <sstream>
// Incluye manipuladores de formato
#include <iomanip>
// Incluye vectores
#include <vector>
// Incluye funciones matematicas
#include <cmath>
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
        // Aplica transformacion potencial
        ptr[i] = cv::saturate_cast<uchar>(std::pow(i / 255.0, gamma) * 255.0);
    }
    // Retorna LUT
    return lut;
}

// Punto de entrada principal
int main() {
    // Directorio base
    std::string carpeta_raiz = "C:/Users/Lenovo/Videos/control/";
    // Carpeta destino
    std::string carpeta_dst = carpeta_raiz + "punto2gamma/";
    // Crea carpeta en disco
    std::filesystem::create_directories(carpeta_dst);

    // Videos base
    std::vector<std::string> lista_videos = {"ideal", "oscuro"};
    // 10 valores seleccionados de gamma
    std::vector<double> valores_gamma = {0.20, 0.35, 0.50, 0.65, 0.80, 1.00, 1.25, 1.50, 1.80, 2.50};
    // Codec de video
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');

    // Recorre ambos videos
    for (const auto& video : lista_videos) {
        // Ruta de entrada
        std::string path_in = carpeta_raiz + video + ".mp4";

        // Itera por los 10 valores de gamma
        for (size_t idx = 0; idx < valores_gamma.size(); ++idx) {
            // Gamma de la prueba actual
            double g = valores_gamma[idx];
            // Precalcula LUT
            cv::Mat lut = crearTablaGamma(g);

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

            // Formatea decimales de gamma
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << g;
            // Prefijo numerico
            std::string prefijo = (idx < 9) ? "exp0" : "exp";
            // Nombre de archivo
            std::string nombre_out = prefijo + std::to_string(idx + 1) + "_" + video + "_gamma_" + ss.str() + ".mp4";
            // Ruta completa
            std::string path_out = carpeta_dst + nombre_out;

            // Instancia grabador monocromatico
            cv::VideoWriter writer(path_out, fourcc, fps, cv::Size(ancho, alto), false);
            // Valida creacion
            if (!writer.isOpened()) { cap.release(); continue; }

            // Notifica en consola
            std::cout << "-> [" << video << "] Gamma = " << ss.str() << "..." << std::flush;

            // Matrices de procesamiento
            cv::Mat frame_bgr, frame_gray, frame_gamma;
            // Procesa 15s
            for (int f = 0; f < total_frames; ++f) {
                // Lee fotograma
                if (!cap.read(frame_bgr)) break;
                // Convierte a grises
                cv::cvtColor(frame_bgr, frame_gray, cv::COLOR_BGR2GRAY);
                // Aplica transformacion
                cv::LUT(frame_gray, lut, frame_gamma);
                // Graba fotograma
                writer.write(frame_gamma);
            }
            // Libera decodificador
            cap.release();
            // Cierra MP4 escribiendo cabecera
            writer.release();
            // Confirma fin
            std::cout << " Listo." << std::endl;
        }
    }
    // Retorna exito
    return 0;
}