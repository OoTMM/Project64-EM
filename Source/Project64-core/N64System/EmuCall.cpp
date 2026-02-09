#include "EmuCall.h"
#include "stdafx.h"
#include "Mips/MemoryVirtualMem.h"

EmuCall::EmuCall(CMipsMemoryVM& memory)
: m_Memory(memory)
{
    for (int i = 0; i < EmuCall::kMaxSockets; ++i)
        m_Sockets[i] = INVALID_SOCKET;
}

void EmuCall::Reset()
{
    for (int i = 0; i < EmuCall::kMaxSockets; ++i)
    {
        if (m_Sockets[i] != INVALID_SOCKET)
        {
            shutdown(m_Sockets[i], SD_BOTH);
            closesocket(m_Sockets[i]);
            m_Sockets[i] = INVALID_SOCKET;
        }
    }
}

static uint32_t swap32(uint32_t value)
{
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0xFF000000) >> 24);
}

void EmuCall::Perform(uint32_t value)
{
    uint32_t op;

    value &= 0x0fffffff;
    if (value > m_Memory.RdramSize() - 4)
        return;
    if (value & 0x03)
        return;

    op = swap32(*(uint32_t*)(m_Memory.Rdram() + value));
    m_Dst = value;

    switch (op)
    {
    case 0: SysCount(); break;
    case 1: SysSocketOpen(); break;
    case 2: SysSocketClose(); break;
    case 3: SysSocketSend(); break;
    case 4: SysSocketRecv(); break;
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
    char* dst;
    uint32_t tmp;

    dst = (char*)m_Memory.Rdram() + m_Dst;
    tmp = swap32(1);
    memcpy(dst, &tmp, 4);
}

void EmuCall::SysSocketOpen()
{
    uint32_t    slot;
    uint32_t*   data;
    SOCKET      sock;

    if (!HasSpace(8))
        return;

    data = (uint32_t*)RamOffset(m_Dst);
    slot = swap32(data[1]);
    if (slot >= kMaxSockets || m_Sockets[slot] != INVALID_SOCKET)
    {
        data[0] = 0xffffffff;
        return;
    }

    /* We want to open a valid socket */
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12500 + slot);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        data[0] = 0xffffffff;
        return;
    }

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) != 0)
    {
        closesocket(sock);
        data[0] = 0xffffffff;
        return;
    }

    /* Set TCP_NODELAY if possible */
    int flag = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

    m_Sockets[slot] = sock;
    data[0] = 0;
}

void EmuCall::SysSocketClose()
{
    uint32_t    slot;
    uint32_t*   data;

    if (!HasSpace(8))
        return;

    data = (uint32_t*)RamOffset(m_Dst);
    slot = swap32(data[1]);
    if (slot >= kMaxSockets || m_Sockets[slot] == INVALID_SOCKET)
    {
        data[0] = 0xffffffff;
        return;
    }

    shutdown(m_Sockets[slot], SD_BOTH);
    closesocket(m_Sockets[slot]);
    m_Sockets[slot] = INVALID_SOCKET;
    data[0] = 0;
}

void EmuCall::SysSocketSend()
{
    uint32_t*   msg;
    uint32_t    slot;
    uint32_t    ptr;
    uint32_t    len;
    SOCKET      sock;
    void*       src;

    if (!HasSpace(16))
        return;

    msg = (uint32_t*)RamOffset(m_Dst);
    slot = swap32(msg[1]);
    ptr = swap32(msg[2]);
    len = swap32(msg[3]);

    if (slot >= kMaxSockets || m_Sockets[slot] == INVALID_SOCKET || len >= 0x100000 || !BoundsCheck(ptr, len))
    {
        msg[0] = 0xffffffff;
        return;
    }

    src = RamOffset(ptr);
    sock = m_Sockets[slot];

    int result = send(sock, (const char*)src, len, 0);
    if (result == SOCKET_ERROR)
        msg[0] = 0xffffffff;
    else
        msg[0] = swap32(result);
}

void EmuCall::SysSocketRecv()
{
    uint32_t*   msg;
    uint32_t    slot;
    uint32_t    ptr;
    uint32_t    len;
    SOCKET      sock;
    void*       dst;

    if (!HasSpace(16))
        return;

    msg = (uint32_t*)RamOffset(m_Dst);
    slot = swap32(msg[1]);
    ptr = swap32(msg[2]);
    len = swap32(msg[3]);

    if (slot >= kMaxSockets || m_Sockets[slot] == INVALID_SOCKET || len >= 0x100000 || !BoundsCheck(ptr, len))
    {
        msg[0] = 0xffffffff;
        return;
    }

    dst = RamOffset(ptr);
    sock = m_Sockets[slot];

    int result = recv(sock, (char*)dst, len, 0);
    if (result == SOCKET_ERROR)
        msg[0] = 0xffffffff;
    else
        msg[0] = swap32(result);
}
