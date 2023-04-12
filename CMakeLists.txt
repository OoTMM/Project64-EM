cmake_minimum_required(VERSION 3.10)

project(Project64-EM)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_compile_definitions(_CRT_SECURE_NO_WARNINGS=1)
add_compile_definitions(UNICODE=1)
add_compile_definitions(_UNICODE=1)
add_compile_definitions(WIN32_LEAN_AND_MEAN=1)

add_subdirectory(Source)
