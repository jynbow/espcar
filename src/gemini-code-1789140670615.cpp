// Incluye OpenCV
#include <opencv2/opencv.hpp>
// Incluye consola estandar
#include <iostream>
// Incluye vectores
#include <vector>

// Punto de entrada principal
int main() {
    // Carpeta base
    std::string carpeta_base = "C:/Users/Lenovo/Videos/control/";
    // Video a procesar
    std::string archivo_video = "ideal.mp4";

    // Puntos fuente del trapecio proyectivo (320x240)
    std::vector<cv::Point2f> pts_src = {
        cv::Point2f(70.0f, 120.0f),  // Sup izq
        cv::Point2f(250.0f, 120.0f), // Sup der
        cv::Point2f(310.0f, 240.0f), // Inf der
        cv::Point2f(10.0f, 240.0f)   // Inf izq
    };

    // Puntos destino rectificados
    std::vector<cv::Point2f> pts_dst = {
        cv::Point2f(40.0f, 0.0f),
        cv::Point2f(280.0f, 0.0f),
        cv::Point2f(280.0f, 240.0f),
        cv::Point2f(40.0f, 240.0f)
    };

    // Calcula matriz de homografia 3x3
    cv::Mat H = cv::getPerspectiveTransform(pts_src, pts_dst);

    // Ruta de entrada
    std::string path_in = carpeta_base + archivo_video;
    // Instancia decodificador
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

    // Codec de video
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    // Ruta de salida
    std::string path_out = carpeta_base + "punto3_perspectiva_bw_" + archivo_video;
    // Grabador monocromatico
    cv::VideoWriter writer(path_out, fourcc, fps, cv::Size(ancho, alto), false);

    // Matrices intermedias
    cv::Mat frame_bgr, frame_gray, frame_warp, frame_roi;
    // Ventanas
    cv::namedWindow("Trapecio Seleccionado", cv::WINDOW_AUTOSIZE);
    // Ventana rectificada
    cv::namedWindow("Perspectiva Rectificada", cv::WINDOW_AUTOSIZE);

    // Procesa 15s
    for (int i = 0; i < total_frames; ++i) {
        // Lee fotograma
        if (!cap.read(frame_bgr)) break;
        // Convierte a grises
        cv::cvtColor(frame_bgr, frame_gray, cv::COLOR_BGR2GRAY);
        // Aplica transformacion proyectiva
        cv::warpPerspective(frame_gray, frame_warp, H, cv::Size(ancho, alto));

        // Graba cuadro rectificado
        writer.write(frame_warp);

        // Clona para trazar lineas guias
        frame_roi = frame_bgr.clone();
        // Dibuja trapecio
        for (int k = 0; k < 4; ++k) {
            // Traza arista
            cv::line(frame_roi, pts_src[k], pts_src[(k + 1) % 4], cv::Scalar(0, 255, 0), 2);
        }

        // Muestra trapecio
        cv::imshow("Trapecio Seleccionado", frame_roi);
        // Muestra resultado
        cv::imshow("Perspectiva Rectificada", frame_warp);

        // Temporizador
        char c = static_cast<char>(cv::waitKey(static_cast<int>(1000.0 / fps)));
        // Sale con ESC
        if (c == 27) break;
    }

    // Libera recursos
    cap.release();
    // Cierra MP4
    writer.release();
    // Destruye ventanas
    cv::destroyAllWindows();

    // Retorna exito
    return 0;
}