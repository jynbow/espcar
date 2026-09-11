// Incluye la biblioteca central de OpenCV para manejo de matrices e imagenes
#include <opencv2/opencv.hpp>
// Incluye funciones estandar para impresion por consola
#include <iostream>

// Punto de entrada principal
int main() {
    // Ruta base a los archivos de video
    std::string carpeta_base = "C:/Users/Lenovo/Videos/control/";
    // Nombre del video a procesar
    std::string archivo_video = "ideal.mp4";
    // Umbral de corte estatico T para binarizacion
    int umbral_corte = 120;

    // Ruta absoluta del video de entrada
    std::string path_in = carpeta_base + archivo_video;
    // Instancia el decodificador de video
    cv::VideoCapture cap(path_in);
    // Valida la lectura del archivo
    if (!cap.isOpened()) {
        // Notifica error en consola
        std::cerr << "Error: No se pudo abrir: " << path_in << std::endl;
        // Retorna con error
        return -1;
    }

    // Obtiene los cuadros por segundo nativos
    double fps = cap.get(cv::CAP_PROP_FPS);
    // Obtiene el ancho en pixeles
    int ancho = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    // Obtiene el alto en pixeles
    int alto = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    // Estructura dimensional de los fotogramas
    cv::Size frame_size(ancho, alto);
    // Cuadros equivalentes a 15 segundos exactos
    int total_frames_15s = static_cast<int>(fps * 15.0);

    // Codec de video compatible con MP4
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    // Ruta del archivo de salida
    std::string path_out = carpeta_base + "punto1_bw_" + archivo_video;
    // Instancia el grabador de video monocromatico
    cv::VideoWriter writer(path_out, fourcc, fps, frame_size, false);

    // Valida la creacion del grabador
    if (!writer.isOpened()) {
        // Notifica error de permisos
        std::cerr << "Error al instanciar VideoWriter: " << path_out << std::endl;
        // Retorna con error
        return -1;
    }

    // Matrices intermedias
    cv::Mat frame_bgr, frame_gray, frame_bw;
    // Crea ventana original
    cv::namedWindow("Original", cv::WINDOW_AUTOSIZE);
    // Crea ventana binarizada
    cv::namedWindow("Binarizado", cv::WINDOW_AUTOSIZE);

    // Procesa los cuadros de los 15 segundos
    for (int i = 0; i < total_frames_15s; ++i) {
        // Lee el cuadro actual
        if (!cap.read(frame_bgr)) {
            // Sale si concluye el video
            break;
        }
        // Convierte el cuadro de color a escala de grises
        cv::cvtColor(frame_bgr, frame_gray, cv::COLOR_BGR2GRAY);
        // Aplica la operacion de binarizacion
        cv::threshold(frame_gray, frame_bw, umbral_corte, 255, cv::THRESH_BINARY);

        // Escribe el cuadro en el archivo MP4
        writer.write(frame_bw);

        // Despliega ventana original
        cv::imshow("Original", frame_bgr);
        // Despliega ventana binarizada
        cv::imshow("Binarizado", frame_bw);

        // Control de temporizacion
        char c = static_cast<char>(cv::waitKey(static_cast<int>(1000.0 / fps)));
        // Finaliza con tecla ESC
        if (c == 27) {
            // Sale del bucle
            break;
        }
    }

    // Libera captura
    cap.release();
    // Vacia buffer y escribe cabecera moov
    writer.release();
    // Destruye ventanas
    cv::destroyAllWindows();

    // Notifica finalizacion
    std::cout << "Video exportado: " << path_out << std::endl;
    // Retorna exito
    return 0;
}