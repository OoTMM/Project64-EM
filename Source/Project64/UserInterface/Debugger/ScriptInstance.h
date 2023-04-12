#pragma once

#include "stdafx.h"

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

typedef enum {
    STATE_STARTED, // Initial evaluation and execution
    STATE_RUNNING, // Event loop running with pending events
    STATE_STOPPED,  // No pending events
    STATE_INVALID
} INSTANCE_STATE;

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

    CDebuggerUI*        _debugger;
    INSTANCE_STATE      _state;
    std::thread         _thread;
    lua_State*          _L;
};
