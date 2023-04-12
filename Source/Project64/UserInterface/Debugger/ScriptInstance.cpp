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

    /* Register memory */
    static const luaL_Reg kRegMemory[] = {
        { "read_u8",   dispatch<&CScriptInstance::API_MemoryRead<uint8_t>>  },
        { "read_u16",  dispatch<&CScriptInstance::API_MemoryRead<uint16_t>> },
        { "read_u32",  dispatch<&CScriptInstance::API_MemoryRead<uint32_t>> },
        { "read_s8",   dispatch<&CScriptInstance::API_MemoryRead<int8_t>>   },
        { "read_s16",  dispatch<&CScriptInstance::API_MemoryRead<int16_t>>  },
        { "read_s32",  dispatch<&CScriptInstance::API_MemoryRead<int32_t>>  },
        { "read_f32",  dispatch<&CScriptInstance::API_MemoryRead<float>>   },
        { "read_f64",  dispatch<&CScriptInstance::API_MemoryRead<double>>  },
        { "write_u8",  dispatch<&CScriptInstance::API_MemoryWrite<uint8_t>>  },
        { "write_u16", dispatch<&CScriptInstance::API_MemoryWrite<uint16_t>> },
        { "write_u32", dispatch<&CScriptInstance::API_MemoryWrite<uint32_t>> },
        { "write_s8",  dispatch<&CScriptInstance::API_MemoryWrite<int8_t>>   },
        { "write_s16", dispatch<&CScriptInstance::API_MemoryWrite<int16_t>>  },
        { "write_s32", dispatch<&CScriptInstance::API_MemoryWrite<int32_t>>  },
        { "write_f32", dispatch<&CScriptInstance::API_MemoryWrite<float>>   },
        { "write_f64", dispatch<&CScriptInstance::API_MemoryWrite<double>>  },
        { nullptr, nullptr }
    };
    luaL_newlib(_L, kRegMemory);
    lua_setglobal(_L, "memory");
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

template <typename T> int CScriptInstance::API_MemoryRead(void)
{
    uint32_t addr;
    T value;

    addr = (uint32_t)luaL_checkinteger(_L, 1);
    _debugger->DebugLoad_VAddr<T>(addr, value);
    lua_pushnumber(_L, value);

    return 1;
}

template <typename T> int CScriptInstance::API_MemoryWrite(void)
{
    uint32_t addr;
    T value;

    addr = (uint32_t)luaL_checkinteger(_L, 1);
    value = (T)luaL_checknumber(_L, 2);
    _debugger->DebugStore_VAddr<T>(addr, value);

    return 0;
}
