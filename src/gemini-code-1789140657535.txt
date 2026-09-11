# Define la version minima requerida de CMake
cmake_minimum_required(VERSION 3.20)
# Define el nombre del proyecto de laboratorio
project(LaboratorioPDI CXX)

# Establece el estandar de C++ a C++17
set(CMAKE_CXX_STANDARD 17)
# Obliga al compilador a cumplir de forma estricta con el estandar C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Localiza el paquete de OpenCV instalado a traves de vcpkg o del sistema
find_package(OpenCV REQUIRED)

# Incluye los directorios de cabeceras de OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})

# Macro auxiliar para declarar un ejecutable y enlazarlo con las librerias de OpenCV
macro(agregar_punto_lab NOMBRE_OBJETIVO ARCHIVO_FUENTE)
    # Agrega el ejecutable a partir de su archivo fuente
    add_executable(${NOMBRE_OBJETIVO} ${ARCHIVO_FUENTE})
    # Enlaza las librerias dinamicas de OpenCV al ejecutable generado
    target_link_libraries(${NOMBRE_OBJETIVO} PRIVATE ${OpenCV_LIBS})
endmacro()

# Declaracion de cada ejecutable independiente
agregar_punto_lab(p1_individual             src/punto1_binarizacion.cpp)
agregar_punto_lab(p1_batch                  src/punto1_batch.cpp)
agregar_punto_lab(p2_individual             src/punto2_gamma.cpp)
agregar_punto_lab(p2_batch                  src/punto2_batch.cpp)
agregar_punto_lab(p3_individual             src/punto3_perspectiva.cpp)
agregar_punto_lab(p3_batch                  src/punto3_batch.cpp)
agregar_punto_lab(p4_individual             src/punto4_filtros_individual.cpp)
agregar_punto_lab(p4_batch                  src/punto4_batch_24exp.cpp)
agregar_punto_lab(p5_individual             src/punto5_histograma.cpp)
agregar_punto_lab(p5_batch                  src/punto5_batch.cpp)
agregar_punto_lab(util_grises               src/utilidad_conversion_grises.cpp)
agregar_punto_lab(analisis_snr              src/analisis_snr_fondo.cpp)
agregar_punto_lab(comparador_mosaico        src/comparador_mosaico.cpp)