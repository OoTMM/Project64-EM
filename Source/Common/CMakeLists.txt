add_definitions(-DUNICODE=1 -D_UNICODE=1)

file(GLOB_RECURSE SOURCES "*.cpp" "*.h")
include_directories("${CMAKE_SOURCE_DIR}/Source/3rdParty" "${CMAKE_SOURCE_DIR}/Source")
add_library(Common STATIC ${SOURCES})
