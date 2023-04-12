cmake_minimum_required(VERSION 3.10)

project(Project64-EM)

set(PJ64_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
make_directory("${PJ64_DIRECTORY}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5208")
add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
add_definitions(-DWIN32_LEAN_AND_MEAN=1)

add_subdirectory(vendor)
add_subdirectory(Source)
