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
    case 1: SysSocketOpen(); break;
    case 2: SysSocketClose(); break;
    case 3: SysSocketSend(); break;
    case 4: SysSocketRecv(); break;
    case 5: SysSocketIsValid(); break;
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

void EmuCall::SysSocketOpen()
{
    uint32_t    slot;
    uint32_t*   pkt;
    SOCKET      sock;
    char        buf[128];

    if (!HasSpace(8))
        return;

    pkt = (uint32_t*)RamOffset(m_Dst);
    slot = pkt[1];
    if (slot >= kMaxSockets)
    {
        pkt[0] = 0xffffffff;
        return;
    }

    /* Socket already open? */
    if (m_Sockets[slot] != INVALID_SOCKET)
    {
        pkt[0] = 0;
        return;
    }

    /* We want to open a valid socket */
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(55630 + slot);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        pkt[0] = 0xffffffff;
        return;
    }

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) != 0)
    {
        int err = WSAGetLastError();
        sprintf(buf, "Failed to connect socket (Error: %d)", err);
        closesocket(sock);
        pkt[0] = 0xffffffff;
        return;
    }

    /* Set TCP_NODELAY if possible */
    int flag = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

    m_Sockets[slot] = sock;
    pkt[0] = 0;
}

void EmuCall::SysSocketClose()
{
    uint32_t    slot;
    uint32_t*   pkt;

    if (!HasSpace(8))
        return;

    pkt = (uint32_t*)RamOffset(m_Dst);
    slot = pkt[1];
    if (slot >= kMaxSockets || m_Sockets[slot] == INVALID_SOCKET)
    {
        pkt[0] = 0xffffffff;
        return;
    }

    shutdown(m_Sockets[slot], SD_BOTH);
    closesocket(m_Sockets[slot]);
    m_Sockets[slot] = INVALID_SOCKET;
    pkt[0] = 0;
}

void EmuCall::SysSocketSend()
{
    uint32_t*   msg;
    uint32_t    slot;
    uint32_t    ptr;
    uint32_t    len;
    SOCKET      sock;
    char*       src;

    if (!HasSpace(16))
        return;

    msg = (uint32_t*)RamOffset(m_Dst);
    slot = msg[1];
    ptr = msg[2] & 0x0fffffff;
    len = msg[3];

    if (slot >= kMaxSockets || m_Sockets[slot] == INVALID_SOCKET || len >= 0x100000 || !BoundsCheck(ptr, len))
    {
        msg[0] = 0xffffffff;
        return;
    }

    /* Copy */
    if (len > 0)
    {
        src = (char*)malloc(len);
        for (uint32_t i = 0; i < len; ++i)
            src[i] = (char)m_Memory.Rdram()[(ptr + i) ^ 3];
    }
    else
    {
        src = nullptr;
    }

    sock = m_Sockets[slot];

    int result = send(sock, src, len, 0);
    if (result == SOCKET_ERROR)
        msg[0] = 0xffffffff;
    else
        msg[0] = (uint32_t)result;
    free(src);
}

void EmuCall::SysSocketRecv()
{
    uint32_t*   msg;
    uint32_t    slot;
    uint32_t    ptr;
    uint32_t    len;
    SOCKET      sock;
    char*       dst;

    if (!HasSpace(16))
        return;

    msg = (uint32_t*)RamOffset(m_Dst);
    slot = msg[1];
    ptr = msg[2] & 0x0fffffff;
    len = msg[3];

    if (slot >= kMaxSockets || m_Sockets[slot] == INVALID_SOCKET || len >= 0x100000 || !BoundsCheck(ptr, len))
    {
        msg[0] = 0xffffffff;
        return;
    }

    if (len)
    {
        dst = (char*)malloc(len);
    }
    else
    {
        dst = nullptr;
    }
    sock = m_Sockets[slot];

    int result = recv(sock, dst, len, 0);

    /* Copy */
    if (result >= 0)
    {
        for (int i = 0; i < result; ++i)
            m_Memory.Rdram()[(ptr + i) ^ 3] = dst[i];
        msg[0] = (uint32_t)result;
    }
    else
        msg[0] = 0xffffffff;
    free(dst);
}

void EmuCall::SysSocketIsValid()
{
    uint32_t*   msg;
    uint32_t    slot;
    SOCKET      sock;

    if (!HasSpace(8))
        return;

    msg = (uint32_t*)RamOffset(m_Dst);
    slot = msg[1];

    if (slot >= kMaxSockets)
    {
        msg[0] = 0xffffffff;
        return;
    }

    sock = m_Sockets[slot];
    if (sock == INVALID_SOCKET)
        msg[0] = 0xffffffff;
    else
        msg[0] = 0;
}
