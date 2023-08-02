add_definitions(-DUNICODE=1 -D_UNICODE=1)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Version.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/Version.h")
file(GLOB_RECURSE SOURCES "*.c" "*.cpp" "*.rc" "*.h")
include_directories("${CMAKE_SOURCE_DIR}/Source/3rdParty" "${CMAKE_SOURCE_DIR}/Source" "${CMAKE_CURRENT_SOURCE_DIR}")
add_library(NRage-v2 SHARED ${SOURCES})
target_link_libraries(NRage-v2 PRIVATE Common Settings comctl32.lib dinput8.lib)

# Copy Plugin
add_custom_command(
  TARGET NRage-v2 POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${PJ64_DIRECTORY}/Plugin/Input"
  COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:NRage-v2> "${PJ64_DIRECTORY}/Plugin/Input"
  COMMENT "Copying NRage-v2"
  VERBATIM
)
