#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#include "sfa/pal/Clock.hpp"
#include "sfa/pal/Socket.hpp"

Result Socket::create(const IPv4Address kIp,
                      const U16 kPort,
                      const Protocol kProto,
                      Socket& kSock)
{
    // Verify socket is not already initialized.
    if (kSock.mFd != -1)
    {
        return E_SOCK_REINIT;
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
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    const U32 ipNetOrder = ((kIp.oct4 << 24) |
                            (kIp.oct3 << 16) |
                            (kIp.oct2 <<  8) |
                            (kIp.oct1 <<  0));
    addr.sin_addr.s_addr = ipNetOrder;
    addr.sin_port = htons(kPort);
    if (bind(fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        return E_SOCK_BIND;
    }

    // If we got this far, socket is ready- set the FD so that its interface
    // is usable.
    kSock.mFd = fd;

    return SUCCESS;
}

Result Socket::select(Socket* const kSocks[],
                      bool* const kReady,
                      const U32 kNumSocks,
                      U32& kTimeoutUs)
{
    // Verify arrays are non-null.
    if ((kSocks == nullptr) || (kReady == nullptr))
    {
        return E_SOCK_NULL;
    }

    // Verify at least 1 socket was provided.
    if (kNumSocks == 0)
    {
        return E_SOCK_SEL_NONE;
    }

    // Verify all sockets are non-null and initialized.
    for (U32 i = 0; i < kNumSocks; ++i)
    {
        if (kSocks[i] == nullptr)
        {
            return E_SOCK_NULL;
        }

        if (kSocks[i]->mFd == -1)
        {
            return E_SOCK_UNINIT;
        }
    }

    // Make FD set.
    fd_set fds;
    FD_ZERO(&fds);
    for (U32 i = 0; i < kNumSocks; ++i)
    {
        if (kReady[i] == false)
        {
            FD_SET(kSocks[i]->mFd, &fds);
        }
    }

    // Make timeout.
    timeval timeout = {0, 0};
    timeout.tv_sec = (kTimeoutUs / Clock::US_IN_S);
    timeout.tv_usec = (kTimeoutUs % Clock::US_IN_S);

    // Do select.
    const I32 selRet = ::select(FD_SETSIZE, &fds, nullptr, nullptr, &timeout);
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
            if (FD_ISSET(kSocks[i]->mFd, &fds) != 0)
            {
                kReady[i] = true;
            }
        }
    }

    // Update timeout return parameter based on how much time remained.
    kTimeoutUs = ((timeout.tv_sec * Clock::US_IN_S) + timeout.tv_usec);

    return SUCCESS;
}

Socket::Socket() : mFd(-1)
{
}

Socket::~Socket()
{
    (void) this->close();
}

Result Socket::send(const IPv4Address kDestIp,
                    const U16 kDestPort,
                    const void* const kBuf,
                    const U32 kNumBytes,
                    U32* const kNumBytesSent)
{
    // Verify socket is initialized.
    if (mFd == -1)
    {
        return E_SOCK_UNINIT;
    }

    // Verify buffer is non-null.
    if (kBuf == nullptr)
    {
        return E_SOCK_NULL;
    }

    // Create destination address.
    sockaddr_in destAddr;
    std::memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    const U32 ipNetOrder = ((kDestIp.oct4 << 24) |
                            (kDestIp.oct3 << 16) |
                            (kDestIp.oct2 <<  8) |
                            (kDestIp.oct1 <<  0));
    destAddr.sin_addr.s_addr = ipNetOrder;
    destAddr.sin_port = htons(kDestPort);

    // Send buffer.
    const I32 bytesSent = sendto(mFd,
                                 kBuf,
                                 kNumBytes,
                                 0,
                                 reinterpret_cast<const sockaddr*>(&destAddr),
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

Result Socket::recv(void* const kBuf,
                    const U32 kNumBytes,
                    U32* const kNumBytesRecvd)
{
    // Verify socket is initialized.
    if (mFd == -1)
    {
        return E_SOCK_UNINIT;
    }

    // Verify buffer is non-null.
    if (kBuf == nullptr)
    {
        return E_SOCK_NULL;
    }

    // Receive into buffer.
    const I32 bytesRecvd = ::recv(mFd,
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

Result Socket::close()
{
    // Verify socket is initialized.
    if (mFd == -1)
    {
        return E_SOCK_UNINIT;
    }

    // Close socket FD.
    if (::close(mFd) != 0)
    {
        return E_SOCK_CLOSE;
    }

    // Reset socket FD to uninitialize socket.
    mFd = -1;

    return SUCCESS;
}
