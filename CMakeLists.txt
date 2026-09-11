cmake_minimum_required(VERSION 3.20)
project(LaboratorioPDI CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

macro(agregar_punto NOMBRE ARCHIVO)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${ARCHIVO})
        add_executable(${NOMBRE} ${ARCHIVO})
        target_link_libraries(${NOMBRE} PRIVATE ${OpenCV_LIBS})
    endif()
endmacro()

agregar_punto(p1 src/punto1_binarizacion.cpp)
agregar_punto(p2 src/punto2_gamma.cpp)
agregar_punto(p3 src/punto3_perspectiva.cpp)
agregar_punto(p4 src/punto4_filtros.cpp)
agregar_punto(p5 src/punto5_histograma.cpp)
