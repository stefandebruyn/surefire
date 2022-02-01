#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "pal/Clock.hpp"
#include "pal/Socket.hpp"

Result Socket::create(const char* const kIp,
                      const U16 kPort,
                      const Protocol kProto,
                      I32& kSock)
{
    // Verify IP is non-null.
    if (kIp == nullptr)
    {
        return E_SOCK_NULLPTR;
    }

    // Map protocol onto corresponding UNIX constant.
    I32 sockType = -1;
    switch (kProto)
    {
        case UDP:
            sockType = SOCK_DGRAM;
            break;

        default:
            return E_SOCK_PROTO;
    }

    // Open socket.
    const I32 fd = socket(AF_INET, sockType, 0);
    if (fd < 0)
    {
        return E_SOCK_OPEN;
    }

    // Bind socket to specified address.
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(kIp);
    addr.sin_port = htons(kPort);
    if (bind(fd, (const sockaddr*) &addr, sizeof(addr)) != 0)
    {
        return E_SOCK_BIND;
    }

    // If we got this far, socket is ready- return FD.
    kSock = fd;
    return SUCCESS;
}

Result Socket::send(const I32 kSock,
                    const char* const kDestIp,
                    const U16 kDestPort,
                    const void* const kBuf,
                    const U32 kNumBytes,
                    U32* const kNumBytesSent)
{
    // Verify destination IP and buffer are non-null.
    if ((kDestIp == nullptr) || (kBuf == nullptr))
    {
        return E_SOCK_NULLPTR;
    }

    // Create destination address.
    sockaddr_in destAddr = {};
    destAddr.sin_family = AF_INET;
    destAddr.sin_addr.s_addr = inet_addr(kDestIp);
    destAddr.sin_port = htons(kDestPort);

    // Send buffer.
    const I32 bytesSent = sendto(kSock,
                                 kBuf,
                                 kNumBytes,
                                 0,
                                 (const sockaddr*) &destAddr,
                                 sizeof(destAddr));
    if (bytesSent < 0)
    {
        // Send failed.
        return E_SOCK_SEND;
    }

    // Return number of bytes sent if caller provided a pointer to do so.
    if (kNumBytesSent != nullptr)
    {
        *kNumBytesSent = bytesSent;
    }

    return SUCCESS;
}

Result Socket::recv(const I32 kSock,
                    void* const kBuf,
                    const U32 kNumBytes,
                    U32* const kNumBytesRecvd)
{
    // Verify buffer is non-null.
    if (kBuf == nullptr)
    {
        return E_SOCK_NULLPTR;
    }

    // Receive into buffer.
    const I32 bytesRecvd = ::recv(kSock,
                                  const_cast<void*>(kBuf),
                                  kNumBytes,
                                  MSG_TRUNC);
    if (bytesRecvd < 0)
    {
        // Receive failed.
        return E_SOCK_RECV;
    }

    // Return number of bytes received if caller provided a pointer to do so.
    if (kNumBytesRecvd != nullptr)
    {
        *kNumBytesRecvd = bytesRecvd;
    }

    return SUCCESS;
}

Result Socket::select(const I32* const kSocks,
                      bool* const kReady,
                      const U32 kNumSocks,
                      U32& kTimeoutUs)
{
    // Verify arrays are non-null.
    if ((kSocks == nullptr) || (kReady == nullptr))
    {
        return E_SOCK_NULLPTR;
    }

    // Check that at least 1 socket was provided.
    if (kNumSocks == 0)
    {
        return E_SOCK_SEL_NONE;
    }

    // Make FD set.
    fd_set fds;
    FD_ZERO(&fds);
    for (U32 i = 0; i < kNumSocks; ++i)
    {
        if (kReady[i] == false)
        {
            FD_SET(kSocks[i], &fds);
        }
    }

    // Make timeout.
    timeval timeout = {};
    timeout.tv_sec = (kTimeoutUs / Clock::US_IN_S);
    timeout.tv_usec = (kTimeoutUs % Clock::US_IN_S);

    // Do select.
    const I32 selRet = select(FD_SETSIZE, &fds, nullptr, nullptr, &timeout);
    if (selRet < 0)
    {
        // Select failed.
        return E_SOCK_SEL;
    }

    if (selRet != 0)
    {
        // Set ready flags according to which sockets have data available.
        for (U32 i = 0; i < kNumSocks; ++i)
        {
            if (FD_ISSET(kSocks[i], &fds) != 0)
            {
                kReady[i] = true;
            }
        }
    }

    // Update timeout return parameter based on how much time remained.
    kTimeoutUs = ((timeout.tv_sec * Clock::US_IN_S) + timeout.tv_usec);

    return SUCCESS;
}

Result Socket::close(const I32 kSock)
{
    if (::close(kSock) != 0)
    {
        return E_SOCK_CLOSE;
    }

    return SUCCESS;
}
