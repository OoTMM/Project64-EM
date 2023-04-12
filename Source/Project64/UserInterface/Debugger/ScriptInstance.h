#pragma once

#include "stdafx.h"
#include <3rdParty/duktape/duktape.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>

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
    CDebuggerUI*        m_Debugger;
    INSTANCE_STATE      m_State;
};
