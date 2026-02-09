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
    void SysSocketOpen();
    void SysSocketClose();
    void SysSocketSend();
    void SysSocketRecv();

    static const uint32_t kMaxSockets = 1;

    CMipsMemoryVM&  m_Memory;

    uint32_t        m_Dst;
    SOCKET          m_Sockets[kMaxSockets];
};
