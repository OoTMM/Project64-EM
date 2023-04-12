#include <stdafx.h>
#include <sys/stat.h>

#include "ScriptInstance.h"
#include "ScriptSystem.h"
#include "Breakpoints.h"

using LuaImpl = int (CScriptInstance::*)(void);

template <LuaImpl func>
static int dispatch(lua_State* L) {
    CScriptInstance* instance = *static_cast<CScriptInstance**>(lua_getextraspace(L));
    return ((*instance).*func)();
}

CScriptInstance::CScriptInstance(CDebuggerUI* debugger)
: _debugger{debugger}
, _state{STATE_INVALID}
, _L{nullptr}
{
}

CScriptInstance::~CScriptInstance()
{

}

INSTANCE_STATE CScriptInstance::GetState() const
{
    return _state;
}

void CScriptInstance::Start(char* path)
{
    /* Create the script env */
    _L = luaL_newstate();
    luaL_openlibs(_L);

    /* Register the API */
    RegisterAPI();

    /* Load the script */
    if (luaL_loadfile(_L, path))
    {
        /* Handle load error */
        _debugger->Debug_LogScriptsWindow(lua_tostring(_L, -1));
        lua_pop(_L, 1);
        _state = STATE_STOPPED;
        return;
    }

    /* Mark the script as started */
    _state = STATE_RUNNING;

    /* Create a thread */
    _thread = std::thread(&CScriptInstance::ThreadEntry, this);
}

void CScriptInstance::ForceStop()
{
}

void CScriptInstance::Eval(const char* code)
{
}

void CScriptInstance::RegisterAPI()
{
    CScriptInstance** extraSpace;

    /* Register the instance */
    extraSpace = (CScriptInstance**)lua_getextraspace(_L);
    *extraSpace = this;

    /* Register print */
    lua_pushcfunction(_L, dispatch<&CScriptInstance::API_Print>);
    lua_setglobal(_L, "print");
}

void CScriptInstance::ThreadEntry()
{
    /* Run the script */
    if (lua_pcall(_L, 0, 0, 0))
    {
        /* Handle errors */
        _debugger->Debug_LogScriptsWindow(lua_tostring(_L, -1));
        lua_pop(_L, 1);
    }

    /* Mark the script as stopped */
    _state = STATE_STOPPED;
}

int CScriptInstance::API_Print(void)
{
    int nargs;

    nargs = lua_gettop(_L);

    for (int i = 1; i <= nargs; ++i)
    {
        /* Convert to string */
        luaL_tolstring(_L, i, nullptr);

        /* Log the string */
        _debugger->Debug_LogScriptsWindow(lua_tostring(_L, -1));

        /* Pop */
        lua_pop(_L, 1);
    }

    return 0;
}
