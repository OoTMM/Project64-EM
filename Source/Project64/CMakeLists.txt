file(GLOB_RECURSE SOURCES "*.cpp" "*.h" "*.rc")
include_directories("${CMAKE_SOURCE_DIR}/Source/3rdParty" "${CMAKE_SOURCE_DIR}/Source" "${CMAKE_SOURCE_DIR}/Source/Project64")
add_executable(Project64-EM WIN32 ${SOURCES})
target_link_libraries(Project64-EM PRIVATE discord-rpc zlib duktape 7zip Common Project64-core)
