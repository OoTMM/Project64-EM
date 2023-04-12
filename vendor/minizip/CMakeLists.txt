include(ExternalProject)

set(MINIZIP_VERSION            3.0.10)
set(MINIZIP_INSTALL_DIR        "${CMAKE_BINARY_DIR}/_vendor")
set(MINIZIP_INCLUDE_DIRECTORY  "${MINIZIP_INSTALL_DIR}/include")
set(MINIZIP_LIBRARY_DIRECTORY  "${MINIZIP_INSTALL_DIR}/lib")

file(MAKE_DIRECTORY "${MINIZIP_INCLUDE_DIRECTORY}")
file(MAKE_DIRECTORY "${MINIZIP_LIBRARY_DIRECTORY}")

set(MINIZIP_IMPORTED_LOCATION          "${MINIZIP_INSTALL_DIR}/lib/libminizip${CMAKE_STATIC_LIBRARY_SUFFIX}")

ExternalProject_Add(
  ep_minizip
  URL                         "https://github.com/zlib-ng/minizip-ng/archive/refs/tags/${MINIZIP_VERSION}.tar.gz"
  DOWNLOAD_EXTRACT_TIMESTAMP  TRUE
  INSTALL_DIR                 "${MINIZIP_INSTALL_DIR}"
  CMAKE_ARGS
    "-DCMAKE_BUILD_TYPE=$<CONFIG>"
    "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
    "-DMZ_ZLIB=ON"
    "-DMZ_BZIP2=OFF"
    "-DMZ_LZMA=OFF"
    "-DMZ_ZSTD=OFF"
    "-DMZ_FETCH_LIBS=OFF"
)
add_dependencies(ep_minizip ZLIB::ZLIB)

add_library(MiniZip::MiniZip STATIC IMPORTED GLOBAL)
set_target_properties(
  MiniZip::MiniZip
  PROPERTIES

  INTERFACE_INCLUDE_DIRECTORIES   "${MINIZIP_INCLUDE_DIRECTORY}"
  IMPORTED_LOCATION               "${MINIZIP_IMPORTED_LOCATION}"
)
add_dependencies(MiniZip::MiniZip ep_minizip)
