file(GLOB_RECURSE SOURCES "*.cpp" "*.h")
include_directories("${CMAKE_SOURCE_DIR}/Source/3rdParty")
add_library(discord-rpc STATIC ${SOURCES})
