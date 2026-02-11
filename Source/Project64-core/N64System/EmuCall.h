#pragma once

#include <cstdint>
#include <winsock2.h>

/* Special system to handle emulator hacks */
class CMipsMemoryVM;
class EmuCall
{
public:
    EmuCall(CMipsMemoryVM& memory);

    void Reset();
    void Perform(uint32_t addr);

private:
    bool  BoundsCheck(uint32_t base, uint32_t size);
    bool  HasSpace(uint32_t size);
    char* RamOffset(uint32_t offset);

    void SysCount();
    void SysValidIPC();
    void SysOpenIPC();
    void SysCloseIPC();
    void SysSendIPC();
    void SysRecvIPC();

    CMipsMemoryVM&  m_Memory;

    uint32_t        m_Dst;
    HANDLE          m_PipePIC;
};
