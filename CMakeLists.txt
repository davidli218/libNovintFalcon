cmake_minimum_required(VERSION 3.30)
project(libNovintFalcon C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/third_party/ftd2xx/include)

link_directories(${PROJECT_SOURCE_DIR}/third_party/ftd2xx/lib)

file(GLOB SRC_FILES "${PROJECT_SOURCE_DIR}/src/*.c")

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} ftd2xx)
