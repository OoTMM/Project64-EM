cmake_minimum_required(VERSION 3.10)
project(lua)

if (WIN32)
    set(CMAKE_DEBUG_POSTFIX d)
endif()

set(SOURCES
    lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c
    lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c lparser.c
    lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c
    lauxlib.c lbaselib.c lcorolib.c ldblib.c liolib.c lmathlib.c
    loadlib.c loslib.c lstrlib.c ltablib.c lutf8lib.c linit.c
)
set(HEADERS lua.h lualib.h lauxlib.h luaconf.h lua.hpp)

list(TRANSFORM SOURCES PREPEND "src/")
list(TRANSFORM HEADERS PREPEND "src/")

add_library(liblua SHARED ${SOURCES})
add_library(liblua_static STATIC ${SOURCES})
target_compile_definitions(liblua PUBLIC LUA_COMPAT_5_3=1)
target_compile_definitions(liblua_static PUBLIC LUA_COMPAT_5_3=1)
if (UNIX)
    target_link_libraries(liblua m)
    target_link_libraries(liblua_static m)
    set_target_properties(liblua PROPERTIES OUTPUT_NAME "lua")
elseif (WIN32)
    set_target_properties(liblua PROPERTIES OUTPUT_NAME "lua54")
    target_compile_definitions(liblua PUBLIC LUA_BUILD_AS_DLL=1)
endif()
add_executable(lua src/lua.c)
add_executable(luac src/luac.c)
target_link_libraries(lua  liblua_static)
target_link_libraries(luac liblua_static)
install(
    TARGETS lua luac liblua
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
)
install(
    FILES ${HEADERS} DESTINATION include
)
