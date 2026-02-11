#include "EmuCall.h"
#include "stdafx.h"
#include "Mips/MemoryVirtualMem.h"

EmuCall::EmuCall(CMipsMemoryVM& memory)
: m_Memory(memory)
, m_PipePIC(INVALID_HANDLE_VALUE)
{

}

void EmuCall::Reset()
{
    if (m_PipePIC != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_PipePIC);
        m_PipePIC = INVALID_HANDLE_VALUE;
    }
}

void EmuCall::Perform(uint32_t value)
{
    uint32_t op;

    value &= 0x0fffffff;
    if (value > m_Memory.RdramSize() - 4)
        return;
    if (value & 0x03)
        return;

    op = *(uint32_t*)(m_Memory.Rdram() + value);
    m_Dst = value;

    switch (op)
    {
    case 0: SysCount(); break;
    case 1: SysValidIPC(); break;
    case 2: SysOpenIPC(); break;
    case 3: SysCloseIPC(); break;
    case 4: SysSendIPC(); break;
    case 5: SysRecvIPC(); break;
    }
}

bool EmuCall::BoundsCheck(uint32_t base, uint32_t size)
{
    uint64_t end = (uint64_t)base + size;
    return (end < m_Memory.RdramSize());
}

bool EmuCall::HasSpace(uint32_t size)
{
    return BoundsCheck(m_Dst, size);
}

char* EmuCall::RamOffset(uint32_t offset)
{
    return (char*)m_Memory.Rdram() + offset;
}

void EmuCall::SysCount()
{
    uint32_t* pkt;

    pkt = (uint32_t*)RamOffset(m_Dst);
    pkt[0] = 6;
}

void EmuCall::SysValidIPC()
{
    uint32_t*   msg;

    msg = (uint32_t*)RamOffset(m_Dst);
    msg[0] = (m_PipePIC != INVALID_HANDLE_VALUE) ? 0 : 0xffffffff;
}

void EmuCall::SysOpenIPC()
{
    uint32_t* msg;
    HANDLE    pipe;

    msg = (uint32_t*)RamOffset(m_Dst);
    if (m_PipePIC != INVALID_HANDLE_VALUE)
    {
        msg[0] = 0xffffffff;
        return;
    }

    pipe = CreateFileA("\\\\.\\pipe\\project64-em", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (pipe == INVALID_HANDLE_VALUE)
    {
        msg[0] = 0xffffffff;
        return;
    }

    m_PipePIC = pipe;
    msg[0] = 0;
}

void EmuCall::SysCloseIPC()
{
    uint32_t*   pkt;

    pkt = (uint32_t*)RamOffset(m_Dst);
    if (m_PipePIC != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_PipePIC);
        m_PipePIC = INVALID_HANDLE_VALUE;
    }
    pkt[0] = 0;
}

void EmuCall::SysSendIPC()
{
    uint32_t* msg;
    char buf[514];
    uint32_t length;
    uint32_t ptr;
    uint16_t length16;

    msg = (uint32_t*)RamOffset(m_Dst);

    /* Basic error checking */
    if (!HasSpace(12) || m_PipePIC == INVALID_HANDLE_VALUE)
    {
        msg[0] = 0;
        return;
    }

    ptr = msg[1] & 0x0fffffff;
    length = msg[2];

    if (length > 512 || !BoundsCheck(ptr, length))
    {
        msg[0] = 0;
        return;
    }

    /* Setup the buffer */
    length16 = htons((uint16_t)length);
    memcpy(buf, &length16, 2);
    for (uint32_t i = 0; i < length; ++i)
        buf[2 + i] = m_Memory.Rdram()[(ptr + i) ^ 3];

    /* Send the data */
    char* bufPtr = buf;
    uint32_t size = length + 2;

    for (;;)
    {
        DWORD bytesWritten;
        if (!WriteFile(m_PipePIC, bufPtr, size, &bytesWritten, nullptr) || bytesWritten == 0)
        {
            CloseHandle(m_PipePIC);
            m_PipePIC = INVALID_HANDLE_VALUE;
            msg[0] = 0;
            return;
        }

        if (bytesWritten == size)
            break;

        bufPtr += bytesWritten;
        size -= bytesWritten;
    }

    msg[0] = length;
}

static int readUntil(HANDLE pipe, char* dst, uint32_t size)
{
    DWORD bytesRead;
    for (;;)
    {
        if (size == 0)
            return 1;
        if (!ReadFile(pipe, dst, size, &bytesRead, nullptr) || bytesRead == 0)
            return 0;
        dst += bytesRead;
        size -= bytesRead;
    }

    return 1;
}

void EmuCall::SysRecvIPC()
{
    char buf[512];
    int32_t* pkt;
    uint32_t maxLength;
    uint32_t ptr;
    uint16_t length;

    pkt = (int32_t*)RamOffset(m_Dst);
    if (!HasSpace(8) || m_PipePIC == INVALID_HANDLE_VALUE)
    {
        pkt[0] = -1;
        return;
    }
    ptr = pkt[1] & 0x0fffffff;
    maxLength = (uint32_t)pkt[2];

    if (maxLength > 512)
        maxLength = 512;

    if (!BoundsCheck(ptr, maxLength))
    {
        pkt[0] = -1;
        return;
    }

    if (!readUntil(m_PipePIC, (char*)&length, 2))
    {
        pkt[0] = -1;
        return;
    }

    length = ntohs(length);
    if (length > maxLength)
    {
        pkt[0] = -1;
        return;
    }

    /* Length is correct, read the message */
    if (!readUntil(m_PipePIC, buf, length))
    {
        pkt[0] = -1;
        return;
    }

    /* Copy the message */
    for (uint32_t i = 0; i < length; ++i)
        m_Memory.Rdram()[(ptr + i) ^ 3] = buf[i];

    pkt[0] = length;
}
