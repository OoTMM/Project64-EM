add_definitions(-DUNICODE=1 -D_UNICODE=1)

# Build
file(GLOB_RECURSE SOURCES "*.cpp" "*.h" "*.rc")
include_directories("${CMAKE_SOURCE_DIR}/Source/3rdParty" "${CMAKE_SOURCE_DIR}/Source" "${CMAKE_SOURCE_DIR}/Source/Project64")
add_executable(Project64-EM WIN32 ${SOURCES})
target_link_libraries(Project64-EM PRIVATE discord-rpc ZLIB::ZLIB MiniZip::MiniZip duktape 7zip Common Project64-core)

# Copy PJ64 & DLLs
add_custom_command(
  TARGET Project64-EM POST_BUILD
  COMMAND
    "${CMAKE_COMMAND}" -E copy
    $<TARGET_FILE:Project64-EM>
    $<TARGET_FILE:ZLIB::ZLIB>
    "${PJ64_DIRECTORY}"
  COMMENT "Copying DLLs"
  VERBATIM
)

# Copy Data
set(DATA_STAMP "${CMAKE_BINARY_DIR}/data.stamp")
file(GLOB_RECURSE DATA_FILES "${CMAKE_SOURCE_DIR}/Plugin/x86/*" "${CMAKE_SOURCE_DIR}/Lang/*")
set(DATA_DEST "${PJ64_DIRECTORY}")

add_custom_command(
  OUTPUT "${DATA_STAMP}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${DATA_DEST}/Plugin"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${DATA_DEST}/Lang"
  COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/Plugin/x86" "${DATA_DEST}/Plugin"
  COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/Lang" "${DATA_DEST}/Lang"
  COMMAND "${CMAKE_COMMAND}" -E touch "${DATA_STAMP}"
  DEPENDS
    ${DATA_FILES}
  COMMENT "Copying data"
  VERBATIM
)
add_custom_target(Project64_Data ALL DEPENDS "${DATA_STAMP}")
