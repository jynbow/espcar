// Incluye OpenCV
#include <opencv2/opencv.hpp>
// Incluye consola
#include <iostream>
// Incluye vectores
#include <vector>
// Incluye limites numericos
#include <limits>
// Incluye biblioteca matematica
#include <cmath>

// Calcula histograma discreto normalizado de 256 niveles
std::vector<double> calcularHistogramaNormalizado(const cv::Mat& src) {
    // Vector con 256 ceros
    std::vector<double> hist(256, 0.0);
    // Total de pixeles
    int total = src.rows * src.cols;
    // Recorre filas
    for (int r = 0; r < src.rows; ++r) {
        // Puntero a fila
        const uchar* ptr = src.ptr<uchar>(r);
        // Recorre columnas
        for (int c = 0; c < src.cols; ++c) {
            // Suma frecuencia
            hist[ptr[c]] += 1.0;
        }
    }
    // Normaliza probabilidades
    for (int i = 0; i < 256; ++i) hist[i] /= static_cast<double>(total);
    // Retorna histograma normalizado
    return hist;
}

// Calcula la distribucion acumulada (CDF)
std::vector<double> calcularCDF(const std::vector<double>& hist) {
    // Vector de CDF
    std::vector<double> cdf(256, 0.0);
    // Acumulador
    double acc = 0.0;
    // Itera por niveles
    for (int i = 0; i < 256; ++i) {
        // Suma acumulada
        acc += hist[i];
        // Asigna en posicion
        cdf[i] = acc;
    }
    // Retorna CDF
    return cdf;
}

// Genera tabla LUT para Histogram Matching
cv::Mat crearLUTMatching(const cv::Mat& src, const std::vector<double>& cdf_ref) {
    // Histograma de origen
    std::vector<double> hist_src = calcularHistogramaNormalizado(src);
    // CDF de origen
    std::vector<double> cdf_src = calcularCDF(hist_src);

    // Matriz de mapeo LUT
    cv::Mat lut(1, 256, CV_8U);
    // Puntero contiguo
    uchar* ptr = lut.ptr();

    // Recorre los 256 niveles de entrada
    for (int r = 0; r < 256; ++r) {
        // Valor CDF actual
        double val = cdf_src[r];
        // Minima diferencia
        double min_diff = std::numeric_limits<double>::max();
        // Nivel seleccionado
        uchar mejor_s = 0;

        // Busca coincidencia en CDF de referencia
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

// Punto de entrada principal
int main() {
    // Directorio de trabajo
    std::string carpeta_base = "C:/Users/Lenovo/Videos/control/";
    // Archivo de video
    std::string archivo_video = "ideal.mp4";

    // Ruta de entrada
    std::string path_in = carpeta_base + archivo_video;
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

    // Almacena fotogramas en memoria
    std::vector<cv::Mat> cuadros;
    cuadros.reserve(total_frames);
    // Matrices de decodificacion
    cv::Mat frame_bgr, frame_gray;
    // Carga fotogramas
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

    // Valida cantidad de cuadros
    if (cuadros.size() < 10) return -1;

    // 5.a: Extrae umbral Otsu del cuadro 10 (indice 9)
    cv::Mat dummy;
    // Calcula Otsu
    double umbral_otsu = cv::threshold(cuadros[9], dummy, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    // Muestra umbral en consola
    std::cout << "[5.a] Umbral extraido del 10mo cuadro: " << umbral_otsu << std::endl;

    // 5.b: Busca el cuadro con minima presencia de linea negra
    int min_pix = std::numeric_limits<int>::max();
    // Indice de referencia
    int idx_ref = 0;
    // Recorre los cuadros
    for (size_t i = 0; i < cuadros.size(); ++i) {
        // Mascara binaria
        cv::Mat mascara;
        // Segmenta linea
        cv::threshold(cuadros[i], mascara, umbral_otsu, 255, cv::THRESH_BINARY_INV);
        // Cuenta pixeles
        int cnt = cv::countNonZero(mascara);
        // Evalua minimo
        if (cnt < min_pix) {
            // Actualiza minimo
            min_pix = cnt;
            // Guarda indice
            idx_ref = static_cast<int>(i);
        }
    }
    // Muestra cuadro optimo en consola
    std::cout << "[5.b] Cuadro con menor linea: #" << idx_ref << " (" << min_pix << " px)" << std::endl;

    // Extrae distribucion de referencia
    std::vector<double> cdf_ref = calcularCDF(calcularHistogramaNormalizado(cuadros[idx_ref]));

    // Codec de video
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    // Salida 5.a
    cv::VideoWriter writer_a(carpeta_base + "punto5_a_umbralizado.mp4", fourcc, fps, cv::Size(ancho, alto), false);
    // Salida 5.c
    cv::VideoWriter writer_c(carpeta_base + "punto5_c_matching.mp4", fourcc, fps, cv::Size(ancho, alto), false);

    // Ventanas
    cv::namedWindow("Original Grises", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("5.a Umbralizado", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("5.c Histogram Matching", cv::WINDOW_AUTOSIZE);

    // Procesa y reproduce simultaneamente
    for (size_t i = 0; i < cuadros.size(); ++i) {
        // Cuadro original
        cv::Mat act = cuadros[i];
        // Matrices destino
        cv::Mat act_bin, act_matched;

        // Binariza con umbral fijo (5.a)
        cv::threshold(act, act_bin, umbral_otsu, 255, cv::THRESH_BINARY);

        // Aplica Histogram Matching (5.c)
        if (static_cast<int>(i) == idx_ref) {
            // Cuadro patron conserva dinamica
            act_matched = act.clone();
        } else {
            // Genera LUT adaptada
            cv::Mat lut = crearLUTMatching(act, cdf_ref);
            // Aplica transformacion
            cv::LUT(act, lut, act_matched);
        }

        // Graba ambos cuadros
        writer_a.write(act_bin);
        writer_c.write(act_matched);

        // Despliega ventanas
        cv::imshow("Original Grises", act);
        cv::imshow("5.a Umbralizado", act_bin);
        cv::imshow("5.c Histogram Matching", act_matched);

        // Temporizador
        char c = static_cast<char>(cv::waitKey(static_cast<int>(1000.0 / fps)));
        // Sale con ESC
        if (c == 27) break;
    }

    // Libera grabadores
    writer_a.release();
    writer_c.release();
    // Destruye ventanas
    cv::destroyAllWindows();

    // Retorna exito
    return 0;
}