#pragma once

#include "stdafx.h"

#include "ScriptInstanceState.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <thread>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

class CScriptSystem;

class CScriptInstance
{
public:
    CScriptInstance(CDebuggerUI* debugger);
    ~CScriptInstance();

    INSTANCE_STATE GetState() const;

    void Start(char* path);
    void ForceStop();
    void Eval(const char* code);

private:
    void RegisterAPI();
    void ThreadEntry();

    int API_Print(void);
    template <typename T> int API_MemoryRead(void);
    template <typename T> int API_MemoryWrite(void);
    int API_SocketSleep(void);
    int API_SocketTcp(void);
    int API_ClassSocketSend(void);
    int API_ClassSocketRecv(void);
    int API_ClassSocketClose(void);

    CDebuggerUI*        _debugger;
    INSTANCE_STATE      _state;
    std::thread         _thread;
    lua_State*          _L;

    int                 _refMetaSocket;
};
