// Incluye la biblioteca central de OpenCV para manejo matricial de imagen y video
#include <opencv2/opencv.hpp>
// Incluye la biblioteca estandar para operaciones de entrada y salida por consola
#include <iostream>
// Incluye el contenedor de vector dinamico
#include <vector>
// Incluye biblioteca matematica para funciones no lineales de potencia y modulo
#include <cmath>

// Funcion auxiliar encargada de generar la tabla de correspondencia LUT para gamma
cv::Mat crearTablaGamma(double gamma) {
    // Instancia una matriz de 1 fila por 256 columnas con enteros sin signo de 8 bits
    cv::Mat lut(1, 256, CV_8U);
    // Puntero de acceso directo a la memoria continua
    uchar* ptr = lut.ptr();
    // Itera por los 256 niveles posibles de gris
    for (int i = 0; i < 256; ++i) {
        // Aplica la transformacion potencial no lineal y recorta saturaciones en 255
        ptr[i] = cv::saturate_cast<uchar>(std::pow(i / 255.0, gamma) * 255.0);
    }
    // Retorna la matriz de correspondencia calculada
    return lut;
}

// Algoritmo espacial del Mejor Vecino sobre una ventana local de 3x3
cv::Mat aplicarMejorVecino(const cv::Mat& src) {
    // Clona la matriz original para preservar intactos sus bordes perimetrales
    cv::Mat dst = src.clone();
    // Obtiene el numero de filas de la imagen
    int filas = src.rows;
    // Obtiene el numero de columnas de la imagen
    int cols = src.cols;

    // Recorre las filas internas evitando salir de los limites fisicos
    for (int y = 1; y < filas - 1; ++y) {
        // Puntero a la fila superior (y - 1)
        const uchar* prev = src.ptr<uchar>(y - 1);
        // Puntero a la fila central evaluada (y)
        const uchar* curr = src.ptr<uchar>(y);
        // Puntero a la fila inferior (y + 1)
        const uchar* next = src.ptr<uchar>(y + 1);
        // Puntero a la fila destino de salida
        uchar* out = dst.ptr<uchar>(y);

        // Recorre las columnas internas evitando los bordes
        for (int x = 1; x < cols - 1; ++x) {
            // Intensidad del pixel central
            uchar centro = curr[x];

            // Vecindad inmediata de los 8 pixeles contiguos
            uchar vecinos[8] = {
                prev[x - 1], prev[x], prev[x + 1],
                curr[x - 1],           curr[x + 1],
                next[x - 1], next[x], next[x + 1]
            };

            // Inicializa la variable de menor variacion con un valor alto
            int min_diff = 1000;
            // Inicializa el candidato de mejor vecino con el valor del centro
            uchar mejor = centro;

            // Busca el vecino con la minima distancia absoluta de intensidad
            for (int k = 0; k < 8; ++k) {
                // Calcula el modulo de la desviacion de nivel de gris
                int diff = std::abs(static_cast<int>(centro) - static_cast<int>(vecinos[k]));
                // Actualiza si encuentra un tono con mayor similitud
                if (diff < min_diff) {
                    // Actualiza la menor distancia encontrada
                    min_diff = diff;
                    // Asigna el nuevo vecino seleccionado
                    mejor = vecinos[k];
                }
            }
            // Asigna el valor del vecino elegido a la imagen de salida
            out[x] = mejor;
        }
    }
    // Retorna la matriz procesada
    return dst;
}

// Funcion auxiliar para superponer una etiqueta de texto con fondo solido
void agregarEtiqueta(cv::Mat& img, const std::string& texto) {
    // Dibuja una barra rectangular superior de fondo negro solido
    cv::rectangle(img, cv::Point(0, 0), cv::Point(img.cols, 24), cv::Scalar(0), cv::FILLED);
    // Imprime el texto identificador en blanco brillante
    cv::putText(img, texto, cv::Point(8, 16), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(255), 1, cv::LINE_AA);
}

// Punto de entrada principal
int main() {
    // =========================================================================
    // CONFIGURACION GENERAL
    // =========================================================================
    // Ruta base donde se alojan los videos
    std::string carpeta_base = "C:/Users/Lenovo/Videos/control/";

    // Video a inspeccionar en el mosaico: "oscuro.mp4" o "ideal.mp4"
    std::string archivo_video = "ideal.mp4";

    // Gamma a evaluar: 0.40 para levantar oscuro.mp4 | 1.50 para oscurecer ideal.mp4
    double gamma_val = (archivo_video == "oscuro.mp4") ? 0.40 : 1.50;
    // =========================================================================

    // Construye la ruta de acceso al archivo fuente
    std::string path_entrada = carpeta_base + archivo_video;
    // Instancia el decodificador de video
    cv::VideoCapture cap(path_entrada);

    // Valida la correcta apertura del archivo
    if (!cap.isOpened()) {
        // Notifica error por consola en caso de fallo
        std::cerr << "Error: No se pudo abrir: " << path_entrada << std::endl;
        // Retorna con codigo de error
        return -1;
    }

    // Metadatos de reproduccion
    double fps = cap.get(cv::CAP_PROP_FPS);
    // Cuadros correspondientes a 15 segundos
    int total_frames_15s = static_cast<int>(fps * 15.0);

    // Construye la tabla LUT de correspondencia gamma
    cv::Mat lut = crearTablaGamma(gamma_val);

    // Nombre de la ventana unificada
    std::string nombre_ventana = "Comparador Multivista: " + archivo_video;
    // Crea la ventana con tamano autoajustable
    cv::namedWindow(nombre_ventana, cv::WINDOW_AUTOSIZE);

    // Matrices intermedias
    cv::Mat frame_bgr;
    cv::Mat frame_gray;
    cv::Mat frame_gamma;
    cv::Mat frame_prom;
    cv::Mat frame_med;
    cv::Mat frame_vec;

    std::cout << "Reproduciendo mosaico comparativo para: " << archivo_video << std::endl;
    std::cout << "Presiona 'ESC' o 'q' para detener la visualizacion." << std::endl;

    // Itera sobre el segmento de 15 segundos
    for (int f = 0; f < total_frames_15s; ++f) {
        // Lee el cuadro actual
        if (!cap.read(frame_bgr)) {
            // Sale si concluye el archivo antes de tiempo
            break;
        }

        // 1. Pasa a escala de grises original
        cv::cvtColor(frame_bgr, frame_gray, cv::COLOR_BGR2GRAY);

        // 2. Aplica la curva de correccion gamma
        cv::LUT(frame_gray, lut, frame_gamma);

        // 3. Aplica los tres algoritmos espaciales
        cv::blur(frame_gamma, frame_prom, cv::Size(3, 3));
        // Aplica filtro de mediana
        cv::medianBlur(frame_gamma, frame_med, 3);
        // Aplica filtro de mejor vecino
        frame_vec = aplicarMejorVecino(frame_gamma);

        // Copias para rotular las etiquetas visuales
        cv::Mat v1 = frame_gray.clone();
        cv::Mat v2 = frame_gamma.clone();
        cv::Mat v3 = frame_prom.clone();
        cv::Mat v4 = frame_med.clone();
        cv::Mat v5 = frame_vec.clone();

        // 4. Agrega los encabezados informativos a cada cuadro
        agregarEtiqueta(v1, "1. Original (Grises)");
        agregarEtiqueta(v2, "2. Solo Gamma (" + std::to_string(gamma_val).substr(0, 4) + ")");
        agregarEtiqueta(v3, "3. Mascara Promedio (3x3)");
        agregarEtiqueta(v4, "4. Filtro Mediana (3x3)");
        agregarEtiqueta(v5, "5. El Mejor Vecino (3x3)");

        // 5. Crea un sexto cuadro informativo para cerrar la grilla rectangular simetrica
        cv::Mat v6 = cv::Mat::zeros(frame_gray.size(), CV_8U);
        agregarEtiqueta(v6, "Panel Informativo");
        cv::putText(v6, "Cuadro #" + std::to_string(f) + "/" + std::to_string(total_frames_15s),
                    cv::Point(20, 80), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255), 1, cv::LINE_AA);
        cv::putText(v6, "Video: " + archivo_video,
                    cv::Point(20, 110), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255), 1, cv::LINE_AA);
        cv::putText(v6, "Gamma: " + std::to_string(gamma_val).substr(0, 4),
                    cv::Point(20, 140), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255), 1, cv::LINE_AA);
        cv::putText(v6, "Presione ESC para salir",
                    cv::Point(20, 190), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(180), 1, cv::LINE_AA);

        // 6. Concatena horizontalmente las dos filas del mosaico
        cv::Mat fila_superior, fila_inferior, mosaico_final;
        // Concatena las vistas 1, 2 y 3 en la fila superior
        cv::hconcat(std::vector<cv::Mat>{v1, v2, v3}, fila_superior);
        // Concatena las vistas 4, 5 y 6 en la fila inferior
        cv::hconcat(std::vector<cv::Mat>{v4, v5, v6}, fila_inferior);
        // Concatena verticalmente ambas filas para formar la matriz 2x3 completa
        cv::vconcat(fila_superior, fila_inferior, mosaico_final);

        // Muestra el panel multivista en la ventana
        cv::imshow(nombre_ventana, mosaico_final);

        // Control de temporizacion sincronizada con la tasa nativa de cuadros
        char tecla = static_cast<char>(cv::waitKey(static_cast<int>(1000.0 / fps)));
        // Termina de inmediato si el usuario pulsa ESC (27) o la tecla 'q'
        if (tecla == 27 || tecla == 'q' || tecla == 'Q') {
            // Sale del bucle
            break;
        }
    }

    // Libera los recursos del decodificador multimedia
    cap.release();
    // Cierra la ventana grafica abierta
    cv::destroyAllWindows();

    // Notifica finalizacion en consola
    std::cout << "Visualizacion en mosaico completada." << std::endl;


    // Retorna codigo de finalizacion exitosa
    return 0;
}