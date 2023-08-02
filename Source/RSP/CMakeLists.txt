configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Version.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/Version.h")
file(GLOB_RECURSE SOURCES "*.c" "*.cpp" "*.h")
include_directories("${CMAKE_SOURCE_DIR}/Source/3rdParty" "${CMAKE_SOURCE_DIR}/Source")
add_library(RSP SHARED ${SOURCES})
target_link_libraries(RSP PRIVATE Common Settings)
set_target_properties(RSP PROPERTIES OUTPUT_NAME "RSP 1.7")

# Copy Plugin
add_custom_command(
  TARGET RSP POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${PJ64_DIRECTORY}/Plugin/RSP"
  COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:RSP> "${PJ64_DIRECTORY}/Plugin/RSP"
  COMMENT "Copying RSP"
  VERBATIM
)
