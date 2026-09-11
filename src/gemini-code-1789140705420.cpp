// Incluye OpenCV
#include <opencv2/opencv.hpp>
// Incluye consola
#include <iostream>
// Incluye biblioteca matematica
#include <cmath>

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
    // Retorna LUT
    return lut;
}

// Algoritmo espacial del Mejor Vecino (3x3)
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
            // Vecindad de 8 pixeles
            uchar vecinos[8] = {
                prev[x - 1], prev[x], prev[x + 1],
                curr[x - 1],           curr[x + 1],
                next[x - 1], next[x], next[x + 1]
            };

            // Menor discrepancia
            int min_diff = 1000;
            // Candidato
            uchar mejor = centro;

            // Busca el vecino con menor distancia absoluta
            for (int k = 0; k < 8; ++k) {
                // Modulo de la desviacion
                int diff = std::abs(static_cast<int>(centro) - static_cast<int>(vecinos[k]));
                // Actualiza si encuentra mejor coincidencia
                if (diff < min_diff) {
                    // Actualiza minima
                    min_diff = diff;
                    // Asigna vecino
                    mejor = vecinos[k];
                }
            }
            // Guarda resultado
            out[x] = mejor;
        }
    }
    // Retorna matriz filtrada
    return dst;
}

// Punto de entrada principal
int main() {
    // Carpeta de datos
    std::string carpeta_base = "C:/Users/Lenovo/Videos/control/";
    // Archivo de video
    std::string archivo_video = "oscuro.mp4";
    // Filtro a probar: "promedio", "mediana" o "mejor_vecino"
    std::string tipo_filtro = "mediana";
    // Apertura del kernel
    int kernel_size = 5;
    // Correccion gamma previa
    double gamma_val = 0.40;

    // Ruta de entrada
    std::string path_in = carpeta_base + archivo_video;
    // Abre decodificador
    cv::VideoCapture cap(path_in);
    // Valida apertura
    if (!cap.isOpened()) return -1;

    // Metadatos
    double fps = cap.get(cv::CAP_PROP_FPS);
    // Ancho
    int ancho = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    // Alto
    int alto = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    // Cuadros para 15s
    int total_frames = static_cast<int>(fps * 15.0);

    // Precalcula tabla LUT
    cv::Mat lut = crearTablaGamma(gamma_val);

    // Codec de video
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    // Ruta de salida
    std::string path_out = carpeta_base + "punto4_" + tipo_filtro + "_k" + std::to_string(kernel_size) + "_" + archivo_video;
    // Grabador monocromatico
    cv::VideoWriter writer(path_out, fourcc, fps, cv::Size(ancho, alto), false);

    // Matrices intermedias
    cv::Mat frame_bgr, frame_gray, frame_gamma, frame_filtrado;
    // Ventanas
    cv::namedWindow("Entrada con Gamma", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Resultado Filtrado", cv::WINDOW_AUTOSIZE);

    // Procesa 15s
    for (int i = 0; i < total_frames; ++i) {
        // Lee fotograma
        if (!cap.read(frame_bgr)) break;
        // Convierte a grises
        cv::cvtColor(frame_bgr, frame_gray, cv::COLOR_BGR2GRAY);
        // Aplica gamma
        cv::LUT(frame_gray, lut, frame_gamma);

        // Aplica filtro seleccionado
        if (tipo_filtro == "promedio") {
            // Mascara promedio
            cv::blur(frame_gamma, frame_filtrado, cv::Size(kernel_size, kernel_size));
        } else if (tipo_filtro == "mediana") {
            // Filtro mediana
            cv::medianBlur(frame_gamma, frame_filtrado, kernel_size);
        } else if (tipo_filtro == "mejor_vecino") {
            // Mejor vecino
            frame_filtrado = aplicarMejorVecino(frame_gamma);
        }

        // Graba fotograma
        writer.write(frame_filtrado);

        // Muestra fotogramas
        cv::imshow("Entrada con Gamma", frame_gamma);
        cv::imshow("Resultado Filtrado", frame_filtrado);

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