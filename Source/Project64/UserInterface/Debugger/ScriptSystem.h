#pragma once

#include <stdafx.h>
#include <3rdParty/duktape/duktape.h>

#include "ScriptInstance.h"

class CScriptSystem
{
public:
    CScriptSystem(CDebuggerUI* debugger);
    ~CScriptSystem();
    // Run a script in its own context/thread
    void RunScript(const char * path);

    // Kill a script context/thread by its path
    void StopScript(const char * path);

    const char* APIScript();

private:
    typedef struct {
        char* path;
        CScriptInstance* scriptInstance;
    } INSTANCE_ENTRY;

    CDebuggerUI* m_Debugger;
    int m_NumCallbacks;
    char* m_APIScript;

    vector<INSTANCE_ENTRY> m_RunningInstances;

    vector<std::string> m_LogData;

    CriticalSection m_CS;

    HDC m_ScreenDC;

public:
    // Returns true if any of the script hooks have callbacks for scriptInstance

    void SetScreenDC(HDC hdc)
    {
        m_ScreenDC = hdc;
    }

    HDC GetScreenDC()
    {
        return m_ScreenDC;
    }

    inline void LogText(const char* text)
    {
        m_LogData.push_back(text);
        m_Debugger->Debug_RefreshScriptsWindow();
    }

    void DeleteStoppedInstances();
    INSTANCE_STATE GetInstanceState(const char* scriptName);
    CScriptInstance* GetInstance(const char* scriptName);
};
