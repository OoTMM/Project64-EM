file(GLOB_RECURSE SOURCES "*.c" "*.h")
include_directories("${CMAKE_SOURCE_DIR}/Source/3rdParty")
add_library(7zip STATIC ${SOURCES})
