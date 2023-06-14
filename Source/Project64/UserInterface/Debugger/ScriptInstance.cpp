#include <stdafx.h>
#include <sys/stat.h>

#include "ScriptInstance.h"
#include "ScriptSystem.h"
#include "Breakpoints.h"

namespace
{

struct Socket
{
    sockaddr_storage addr;
};

}

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

    /* Register socket class  */
    static const luaL_Reg kRegClassSocket[] = {
        { "send", dispatch<&CScriptInstance::API_ClassSocketSend> },
        { "recv", dispatch<&CScriptInstance::API_ClassSocketRecv> },
        { "close", dispatch<&CScriptInstance::API_ClassSocketClose> },
        { nullptr, nullptr }
    };
    luaL_newmetatable(_L, "socket");
    lua_pushnil(_L);
    lua_setfield(_L, -2, "__metatable");
    luaL_newlib(_L, kRegClassSocket);
    lua_setfield(_L, -2, "__index");
    lua_pop(_L, 1);

    /* Register socket */
    static const luaL_Reg kRegSocket[] = {
        { "sleep", dispatch<&CScriptInstance::API_SocketSleep> },
        { "tcp", dispatch<&CScriptInstance::API_SocketTcp> },
        { nullptr, nullptr }
    };
    luaL_newlib(_L, kRegSocket);
    lua_setglobal(_L, "socket");

    /* Register binary */
    static const luaL_Reg kRegBinary[] = {
        { "pack_u8",    dispatch<&CScriptInstance::API_BinaryPack<uint8_t>>      },
        { "pack_u16",   dispatch<&CScriptInstance::API_BinaryPack<uint16_t>>     },
        { "pack_u32",   dispatch<&CScriptInstance::API_BinaryPack<uint32_t>>     },
        { "pack_s8",    dispatch<&CScriptInstance::API_BinaryPack<int8_t>>       },
        { "pack_s16",   dispatch<&CScriptInstance::API_BinaryPack<int16_t>>      },
        { "pack_s32",   dispatch<&CScriptInstance::API_BinaryPack<int32_t>>      },
        { "pack_f32",   dispatch<&CScriptInstance::API_BinaryPack<float>>        },
        { "pack_f64",   dispatch<&CScriptInstance::API_BinaryPack<double>>       },
        { "unpack_u8",  dispatch<&CScriptInstance::API_BinaryUnpack<uint8_t>>    },
        { "unpack_u16", dispatch<&CScriptInstance::API_BinaryUnpack<uint16_t>>   },
        { "unpack_u32", dispatch<&CScriptInstance::API_BinaryUnpack<uint32_t>>   },
        { "unpack_s8",  dispatch<&CScriptInstance::API_BinaryUnpack<int8_t>>     },
        { "unpack_s16", dispatch<&CScriptInstance::API_BinaryUnpack<int16_t>>    },
        { "unpack_s32", dispatch<&CScriptInstance::API_BinaryUnpack<int32_t>>    },
        { "unpack_f32", dispatch<&CScriptInstance::API_BinaryUnpack<float>>      },
        { "unpack_f64", dispatch<&CScriptInstance::API_BinaryUnpack<double>>     },
        { nullptr, nullptr }
    };
    luaL_newlib(_L, kRegBinary);
    lua_setglobal(_L, "binary");
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

int CScriptInstance::API_SocketSleep(void)
{
    double seconds;

    seconds = luaL_checknumber(_L, 1);
    Sleep((DWORD)(seconds * 1000.0));

    return 0;
}

int CScriptInstance::API_SocketTcp(void)
{
    SOCKET fd;
    const char* host;
    char        portStr[16];
    uint16_t    port;
    addrinfo    hints{};
    addrinfo*   result;
    addrinfo*   info;

    /* Get params */
    host = luaL_checkstring(_L, 1);
    port = (uint16_t)luaL_checkinteger(_L, 2);

    if (port < 0 || port > 65535)
    {
        lua_pushnil(_L);
        return 1;
    }

    /* Get port string */
    sprintf_s(portStr, "%d", port);

    /* Resolve host */
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if (getaddrinfo(host, portStr, &hints, &result))
    {
        lua_pushnil(_L);
        return 1;
    }

    /* Try to connect in sequence */
    info = result;
    fd = INVALID_SOCKET;
    for (;;)
    {
        fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (fd != INVALID_SOCKET)
        {
            if (connect(fd, info->ai_addr, info->ai_addrlen) == 0)
                break;

            closesocket(fd);
            fd = INVALID_SOCKET;
        }
        info = info->ai_next;
        if (!info)
            break;
    }
    freeaddrinfo(result);

    if (fd == INVALID_SOCKET)
    {
        lua_pushnil(_L);
        return 1;
    }

    /* We have a good socket */
    SOCKET* socketPtr = (SOCKET*)lua_newuserdata(_L, sizeof(SOCKET));
    *socketPtr = fd;
    luaL_getmetatable(_L, "socket");
    lua_setmetatable(_L, -2);

    return 1;
}

int CScriptInstance::API_ClassSocketSend(void)
{
    SOCKET sock;
    const char* payload;
    size_t payloadLen;

    sock = *(SOCKET*)luaL_checkudata(_L, 1, "socket");
    payload = luaL_checklstring(_L, 2, &payloadLen);
    send(sock, payload, payloadLen, 0);

    return 0;
}

int CScriptInstance::API_ClassSocketRecv(void)
{
    SOCKET sock;
    int size;
    int recvSize;
    int ret;
    char tmp[1024];
    luaL_Buffer b;

    sock = *(SOCKET*)luaL_checkudata(_L, 1, "socket");
    size = (int)luaL_checkinteger(_L, 2);

    luaL_buffinit(_L, &b);
    while (size)
    {
        recvSize = size > sizeof(tmp) ? sizeof(tmp) : size;
        ret = recv(sock, tmp, recvSize, 0);
        if (ret > 0)
        {
            luaL_addlstring(&b, tmp, ret);
            size -= ret;
        }
    }
    luaL_pushresult(&b);
    return 1;
}

int CScriptInstance::API_ClassSocketClose(void)
{
    SOCKET sock;

    sock = *(SOCKET*)luaL_checkudata(_L, 1, "socket");
    closesocket(sock);
    return 0;
}

template <typename T> int CScriptInstance::API_BinaryPack(void)
{
    T       value;
    char    buffer[sizeof(T)];

    value = (T)luaL_checknumber(_L, 1);
    memcpy(buffer, &value, sizeof(T));
    lua_pushlstring(_L, buffer, sizeof(T));

    return 1;
}

template <typename T> int CScriptInstance::API_BinaryUnpack(void)
{
    const char* str;
    T           value;

    str = luaL_checkstring(_L, 1);
    memcpy(&value, str, sizeof(T));
    lua_pushnumber(_L, lua_Number(value));

    return 1;
}
