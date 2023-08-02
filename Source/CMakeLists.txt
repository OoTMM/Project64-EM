# Build
add_subdirectory(3rdParty)
add_subdirectory(Common)
add_subdirectory(Settings)
add_subdirectory(Project64-core)
add_subdirectory(Project64)
add_subdirectory(RSP)
add_subdirectory(Project64-input)
add_subdirectory(Project64-audio)
add_subdirectory(nragev20)

set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
include(InstallRequiredSystemLibraries)
install(DIRECTORY "${PJ64_DIRECTORY}/" DESTINATION .)
