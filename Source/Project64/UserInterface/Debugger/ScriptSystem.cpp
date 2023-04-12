#include <stdafx.h>
#include "ScriptSystem.h"
#include "Debugger-Scripts.h"

#include "ScriptInstance.h"

CScriptSystem::CScriptSystem(CDebuggerUI* debugger)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    m_Debugger = debugger;

    HMODULE hInst = GetModuleHandle(nullptr);
    HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(IDR_JSAPI_TEXT), L"TEXT");

    HGLOBAL hGlob = LoadResource(hInst, hRes);
    DWORD resSize = SizeofResource(hInst, hRes);
    m_APIScript = (char*)malloc(resSize + 1);

    void* resData = LockResource(hGlob);
    memcpy(m_APIScript, resData, resSize);
    m_APIScript[resSize] = '\0';
    FreeResource(hGlob);
}

CScriptSystem::~CScriptSystem()
{
    free(m_APIScript);
}

const char* CScriptSystem::APIScript()
{
    return m_APIScript;
}

void CScriptSystem::RunScript(const char* path)
{
    CGuard guard(m_CS);
    CScriptInstance* scriptInstance = new CScriptInstance(m_Debugger);
    char* pathSaved = (char*)malloc(strlen(path)+1); // Freed via DeleteStoppedInstances
    strcpy(pathSaved, path);

    m_RunningInstances.push_back({ pathSaved, scriptInstance });
    scriptInstance->Start(pathSaved);
}

void CScriptSystem::StopScript(const char* path)
{
    CScriptInstance* scriptInstance = GetInstance(path);

    if (scriptInstance == nullptr)
    {
        return;
    }

    scriptInstance->ForceStop();
    DeleteStoppedInstances();
}

void CScriptSystem::DeleteStoppedInstances()
{
    CGuard guard(m_CS);

    int lastIndex = m_RunningInstances.size() - 1;
    for (int i = lastIndex; i >= 0; i--)
    {
        if (m_RunningInstances[i].scriptInstance->GetState() == STATE_STOPPED)
        {
            free(m_RunningInstances[i].path);
            CScriptInstance* instance = m_RunningInstances[i].scriptInstance;
            delete instance;
            m_RunningInstances.erase(m_RunningInstances.begin() + i);
        }
    }
}

INSTANCE_STATE CScriptSystem::GetInstanceState(const char* path)
{
    CGuard guard(m_CS);

    for (size_t i = 0; i < m_RunningInstances.size(); i++)
    {
        if (strcmp(m_RunningInstances[i].path, path) == 0)
        {
            INSTANCE_STATE ret = m_RunningInstances[i].scriptInstance->GetState();
            return ret;
        }
    }

    return STATE_INVALID;
}

CScriptInstance* CScriptSystem::GetInstance(const char* path)
{
    CGuard guard(m_CS);

    for (size_t i = 0; i < m_RunningInstances.size(); i++)
    {
        if (strcmp(m_RunningInstances[i].path, path) == 0)
        {
            CScriptInstance *ret = m_RunningInstances[i].scriptInstance;
            return ret;
        }
    }

    return nullptr;
}
