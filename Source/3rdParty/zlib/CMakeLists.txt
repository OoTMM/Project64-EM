set(
  SOURCES
  "adler32.c"
  "compress.c"
  "crc32.c"
  "deflate.c"
  "gzclose.c"
  "gzlib.c"
  "gzread.c"
  "gzwrite.c"
  "infback.c"
  "inffast.c"
  "inflate.c"
  "inftrees.c"
  "trees.c"
  "uncompr.c"
  "zutil.c"
  "contrib/minizip/ioapi.c"
  "contrib/minizip/iowin32.c"
  "contrib/minizip/mztools.c"
  "contrib/minizip/unzip.c"
  "contrib/minizip/zip.c"
)
include_directories("${CMAKE_SOURCE_DIR}/Source/3rdParty" "${CMAKE_SOURCE_DIR}/Source/3rdParty/zlib")
add_library(zlib STATIC ${SOURCES})
