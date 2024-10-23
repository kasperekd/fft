cmake_minimum_required(VERSION 3.10)

project(SDR_FFT_Project)
set(CMAKE_C_STANDARD 99)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${SOURCE_DIR}/include)
set(SOCKET_DIR ${SOURCE_DIR}/my_socket)

include_directories(${INCLUDE_DIR})

add_library(my_socket SHARED ${SOCKET_DIR}/my_socket.c)

set(SOURCES
    ${SOURCE_DIR}/main.c
)

add_executable(main ${SOURCES})

target_link_libraries(main my_socket m)