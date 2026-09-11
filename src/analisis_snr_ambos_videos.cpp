// Incluye OpenCV
#include <opencv2/opencv.hpp>
// Incluye consola
#include <iostream>
// Incluye vectores
#include <vector>
// Incluye sistema de archivos
#include <filesystem>

// Estructura de trapecio
struct TrapecioROI {
    // Vertices del trapecio
    cv::Point2f p1, p2, p3, p4;
};

// Punto de entrada principal
int main() {
    // Directorio raiz
    std::string carpeta_raiz = "C:/Users/Lenovo/Videos/control/";
    // Carpeta destino
    std::string carpeta_dst = carpeta_raiz + "punto3pespectiva/";
    // Crea carpeta en disco
    std::filesystem::create_directories(carpeta_dst);

    // Videos base
    std::vector<std::string> lista_videos = {"ideal", "oscuro"};

    // Puntos destino rectificados
    std::vector<cv::Point2f> pts_dst = {
        cv::Point2f(40.0f, 0.0f),
        cv::Point2f(280.0f, 0.0f),
        cv::Point2f(280.0f, 240.0f),
        cv::Point2f(40.0f, 240.0f)
    };

    // Genera 10 trapecios variando la profundidad
    std::vector<TrapecioROI> lista_trapecios;
    // Itera 10 configuraciones
    for (int k = 0; k < 10; ++k) {
        // Altura vertical
        float y_top = 90.0f + static_cast<float>(k * 6);
        // Margen izquierdo
        float x_left = 50.0f + static_cast<float>(k * 4);
        // Margen derecho
        float x_right = 320.0f - x_left;
        // Agrega a la lista
        lista_trapecios.push_back({
            cv::Point2f(x_left, y_top),
            cv::Point2f(x_right, y_top),
            cv::Point2f(310.0f, 240.0f),
            cv::Point2f(10.0f, 240.0f)
        });
    }

    // Codec de video
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');

    // Recorre ambos videos
    for (const auto& video : lista_videos) {
        // Ruta de entrada
        std::string path_in = carpeta_raiz + video + ".mp4";

        // Itera por los 10 trapecios
        for (size_t idx = 0; idx < lista_trapecios.size(); ++idx) {
            // Trapecio actual
            TrapecioROI t = lista_trapecios[idx];
            // Puntos fuente
            std::vector<cv::Point2f> pts_src = {t.p1, t.p2, t.p3, t.p4};
            // Calcula homografia
            cv::Mat H = cv::getPerspectiveTransform(pts_src, pts_dst);

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

            // Prefijo numerico
            std::string prefijo = (idx < 9) ? "exp0" : "exp";
            // Nombre de archivo
            std::string nombre_out = prefijo + std::to_string(idx + 1) + "_" + video + "_homografia_ytop_" + std::to_string(static_cast<int>(t.p1.y)) + ".mp4";
            // Ruta final
            std::string path_out = carpeta_dst + nombre_out;

            // Grabador monocromatico
            cv::VideoWriter writer(path_out, fourcc, fps, cv::Size(ancho, alto), false);
            // Valida grabador
            if (!writer.isOpened()) { cap.release(); continue; }

            // Notifica en consola
            std::cout << "-> [" << video << "] Y_top = " << t.p1.y << "..." << std::flush;

            // Matrices intermedias
            cv::Mat frame_bgr, frame_gray, frame_warp;
            // Procesa 15s
            for (int f = 0; f < total_frames; ++f) {
                // Lee fotograma
                if (!cap.read(frame_bgr)) break;
                // Convierte a grises
                cv::cvtColor(frame_bgr, frame_gray, cv::COLOR_BGR2GRAY);
                // Aplica transformacion proyectiva
                cv::warpPerspective(frame_gray, frame_warp, H, cv::Size(ancho, alto));
                // Graba fotograma
                writer.write(frame_warp);
            }
            // Libera captura
            cap.release();
            // Cierra MP4
            writer.release();
            // Confirma fin
            std::cout << " Listo." << std::endl;
        }
    }
    // Retorna exito
    return 0;
}