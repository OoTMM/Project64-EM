file(GLOB_RECURSE SOURCES "*.cpp" "*.h")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Version.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/Version.h")
include_directories("${CMAKE_SOURCE_DIR}/Source/3rdParty" "${CMAKE_SOURCE_DIR}/Source")
add_library(Project64-core STATIC ${SOURCES})
