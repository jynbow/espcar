// Incluye OpenCV
#include <opencv2/opencv.hpp>
// Incluye impresion por consola
#include <iostream>
// Incluye vectores dinamicos
#include <vector>
// Incluye gestion de sistema de archivos
#include <filesystem>

// Punto de entrada principal
int main() {
    // Directorio raiz
    std::string carpeta_raiz = "C:/Users/Lenovo/Videos/control/";
    // Carpeta destino del lote
    std::string carpeta_dst = carpeta_raiz + "punto1bw/";
    // Crea el directorio en disco si no existe
    std::filesystem::create_directories(carpeta_dst);

    // Lista de videos a procesar
    std::vector<std::string> lista_videos = {"ideal", "oscuro"};
    // Vector con 10 umbrales de prueba
    std::vector<int> umbrales = {25, 50, 75, 100, 125, 150, 175, 200, 225, 240};
    // Codec de video
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');

    // Recorre los dos videos
    for (const auto& video : lista_videos) {
        // Ruta de entrada
        std::string path_in = carpeta_raiz + video + ".mp4";

        // Itera por los 10 umbrales
        for (size_t idx = 0; idx < umbrales.size(); ++idx) {
            // Umbral actual
            int T = umbrales[idx];
            // Instancia captura
            cv::VideoCapture cap(path_in);
            // Valida apertura
            if (!cap.isOpened()) break;

            // Parametros del video
            double fps = cap.get(cv::CAP_PROP_FPS);
            // Ancho
            int ancho = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
            // Alto
            int alto = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
            // Cuadros para 15 segundos
            int total_frames = static_cast<int>(fps * 15.0);

            // Prefijo numerico formateado
            std::string prefijo = (idx < 9) ? "exp0" : "exp";
            // Nombre de archivo
            std::string nombre_out = prefijo + std::to_string(idx + 1) + "_" + video + "_bw_T" + std::to_string(T) + ".mp4";
            // Ruta final
            std::string path_out = carpeta_dst + nombre_out;

            // Grabador monocromatico
            cv::VideoWriter writer(path_out, fourcc, fps, cv::Size(ancho, alto), false);
            // Valida creacion
            if (!writer.isOpened()) { cap.release(); continue; }

            // Notifica en consola
            std::cout << "-> [" << video << "] Umbral T = " << T << "..." << std::flush;

            // Matrices de trabajo
            cv::Mat frame_bgr, frame_gray, frame_bw;
            // Procesa 15s
            for (int f = 0; f < total_frames; ++f) {
                // Lee fotograma
                if (!cap.read(frame_bgr)) break;
                // Convierte a grises
                cv::cvtColor(frame_bgr, frame_gray, cv::COLOR_BGR2GRAY);
                // Binariza
                cv::threshold(frame_gray, frame_bw, T, 255, cv::THRESH_BINARY);
                // Graba fotograma
                writer.write(frame_bw);
            }
            // Libera decodificador
            cap.release();
            // Cierra contenedor MP4 limpiamente
            writer.release();
            // Confirma fin de prueba
            std::cout << " Listo." << std::endl;
        }
    }
    // Retorna ejecucion exitosa
    return 0;
}