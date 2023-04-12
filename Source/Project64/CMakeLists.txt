# Build
file(GLOB_RECURSE SOURCES "*.cpp" "*.h" "*.rc")
include_directories("${CMAKE_SOURCE_DIR}/Source/3rdParty" "${CMAKE_SOURCE_DIR}/Source" "${CMAKE_SOURCE_DIR}/Source/Project64")
add_executable(Project64-EM WIN32 ${SOURCES})
target_link_libraries(Project64-EM PRIVATE discord-rpc ZLIB::ZLIB MiniZip::MiniZip duktape 7zip Common Project64-core)

# Copy DLLs
add_custom_command(
  TARGET Project64-EM POST_BUILD
  COMMAND
    "${CMAKE_COMMAND}" -E copy
    $<TARGET_FILE:ZLIB::ZLIB>
    $<TARGET_FILE_DIR:Project64-EM>
  COMMENT "Copying DLLs"
  VERBATIM
)
