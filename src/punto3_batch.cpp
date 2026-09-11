// Incluye OpenCV
#include <opencv2/opencv.hpp>
// Incluye consola estandar
#include <iostream>
// Incluye biblioteca matematica
#include <cmath>

// Genera la tabla LUT para la transformacion gamma
cv::Mat crearTablaGamma(double gamma) {
    // Matriz de correspondencia de 256 niveles
    cv::Mat lut(1, 256, CV_8U);
    // Puntero contiguo
    uchar* ptr = lut.ptr();
    // Itera por los niveles
    for (int i = 0; i < 256; ++i) {
        // Aplica curva potencial saturando en 255
        ptr[i] = cv::saturate_cast<uchar>(std::pow(i / 255.0, gamma) * 255.0);
    }
    // Retorna tabla calculada
    return lut;
}

// Punto de entrada principal
int main() {
    // Directorio de trabajo
    std::string carpeta_base = "C:/Users/Lenovo/Videos/control/";
    // Archivo de video
    std::string archivo_video = "oscuro.mp4";
    // Valor de gamma a aplicar
    double gamma_val = 0.40;

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

    // Precomputa LUT
    cv::Mat lut = crearTablaGamma(gamma_val);

    // Codec de video
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    // Ruta de salida
    std::string path_out = carpeta_base + "punto2_gamma_" + archivo_video;
    // Grabador monocromatico
    cv::VideoWriter writer(path_out, fourcc, fps, cv::Size(ancho, alto), false);

    // Matrices intermedias
    cv::Mat frame_bgr, frame_gray, frame_gamma;
    // Ventana grises
    cv::namedWindow("Entrada (Grises)", cv::WINDOW_AUTOSIZE);
    // Ventana gamma
    cv::namedWindow("Correccion Gamma", cv::WINDOW_AUTOSIZE);

    // Procesa 15s
    for (int i = 0; i < total_frames; ++i) {
        // Lee fotograma
        if (!cap.read(frame_bgr)) break;
        // Convierte a grises
        cv::cvtColor(frame_bgr, frame_gray, cv::COLOR_BGR2GRAY);
        // Aplica gamma mediante LUT
        cv::LUT(frame_gray, lut, frame_gamma);

        // Graba fotograma
        writer.write(frame_gamma);

        // Muestra entrada
        cv::imshow("Entrada (Grises)", frame_gray);
        // Muestra salida
        cv::imshow("Correccion Gamma", frame_gamma);

        // Control de temporizacion
        char c = static_cast<char>(cv::waitKey(static_cast<int>(1000.0 / fps)));
        // Sale con ESC
        if (c == 27) break;
    }

    // Libera recursos
    cap.release();
    // Cierra MP4 de forma segura
    writer.release();
    // Destruye ventanas
    cv::destroyAllWindows();

    // Retorna exito
    return 0;
}