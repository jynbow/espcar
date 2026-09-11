// Incluye OpenCV
#include <opencv2/opencv.hpp>
// Incluye consola
#include <iostream>

// Punto de entrada principal
int main() {
    // Ruta del video de entrada
    std::string path_in  = "C:/Users/Lenovo/Videos/control/ideal.mp4";
    // Ruta del video convertido a escala de grises
    std::string path_out = "C:/Users/Lenovo/Videos/control/ideal_grises.mp4";

    // Instancia lector
    cv::VideoCapture cap(path_in);
    // Valida lectura
    if (!cap.isOpened()) return -1;

    // Metadatos
    double fps = cap.get(cv::CAP_PROP_FPS);
    // Ancho
    int ancho = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    // Alto
    int alto = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    // Total de cuadros
    int total = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));

    // Codec de video
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    // Grabador monocromatico
    cv::VideoWriter writer(path_out, fourcc, fps, cv::Size(ancho, alto), false);
    // Valida grabador
    if (!writer.isOpened()) return -1;

    // Matrices de trabajo
    cv::Mat frame_bgr, frame_gray;
    // Contador de cuadros
    int procesados = 0;

    // Convierte el archivo completo
    while (cap.read(frame_bgr)) {
        // Pasa a escala de grises
        cv::cvtColor(frame_bgr, frame_gray, cv::COLOR_BGR2GRAY);
        // Escribe fotograma
        writer.write(frame_gray);
        // Incrementa conteo
        procesados++;
        // Muestra avance cada 30 cuadros
        if (procesados % 30 == 0) {
            // Imprime porcentaje
            std::cout << "\rProcesando: " << procesados << "/" << total << std::flush;
        }
    }

    // Libera recursos
    cap.release();
    // Cierra MP4
    writer.release();
    // Confirma conclusion
    std::cout << "\nConversion completada." << std::endl;

    // Retorna exito
    return 0;
}