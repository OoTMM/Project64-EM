#include <stdafx.h>
#include <sys/stat.h>

#include "ScriptInstance.h"
#include "ScriptSystem.h"
#include "Breakpoints.h"

CScriptInstance::CScriptInstance(CDebuggerUI* debugger)
: m_Debugger{debugger}
, m_State{STATE_INVALID}
{
}

CScriptInstance::~CScriptInstance()
{

}

INSTANCE_STATE CScriptInstance::GetState() const
{
    return m_State;
}

void CScriptInstance::Start(char* path)
{
}

void CScriptInstance::ForceStop()
{
}

void CScriptInstance::Eval(const char* code)
{
}
