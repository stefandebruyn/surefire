////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include <arpa/inet.h>
#include <cstring>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "sf/pal/Clock.hpp"
#include "sf/pal/Socket.hpp"

Result Socket::init(const Ipv4Address kIp,
                    const U16 kPort,
                    const Protocol kProto,
                    Socket& kSock)
{
    // Check that socket is not already initialized.
    if (kSock.mInit)
    {
        return E_SOK_REINIT;
    }

    // Map protocol onto corresponding UNIX constant.
    I32 sockType = -1;
    switch (kProto)
    {
        case UDP:
            sockType = SOCK_DGRAM;
            break;

        default:
            return E_SOK_PROTO;
    }

    // Open socket.
    const I32 fd = socket(AF_INET, sockType, 0);
    if (fd < 0)
    {
        return E_SOK_OPEN;
    }

    // Bind socket to specified address.
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    const U32 ipNetOrder = ((static_cast<U32>(kIp.oct4) << 24) |
                            (static_cast<U32>(kIp.oct3) << 16) |
                            (static_cast<U32>(kIp.oct2) <<  8) |
                            (static_cast<U32>(kIp.oct1) <<  0));
    addr.sin_addr.s_addr = ipNetOrder;
    addr.sin_port = htons(kPort);
    if (bind(fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        return E_SOK_BIND;
    }

    // Socket is ready - initialize it and set FD.
    kSock.mInit = true;
    kSock.mFd = fd;

    return SUCCESS;
}

Result Socket::select(Socket* const kSocks[],
                      bool* const kReady,
                      const U32 kNumSocks,
                      U32& kTimeoutUs)
{
    // Check that arrays are non-null.
    if ((kSocks == nullptr) || (kReady == nullptr))
    {
        return E_SOK_NULL;
    }

    // Check that at least 1 socket was provided.
    if (kNumSocks == 0)
    {
        return E_SOK_SEL_NONE;
    }

    // Check that all sockets are non-null and initialized.
    for (U32 i = 0; i < kNumSocks; ++i)
    {
        if (kSocks[i] == nullptr)
        {
            return E_SOK_NULL;
        }

        if (!kSocks[i]->mInit)
        {
            return E_SOK_UNINIT;
        }
    }

    // Make FD set.
    fd_set fds;
    FD_ZERO(&fds);
    for (U32 i = 0; i < kNumSocks; ++i)
    {
        if (!kReady[i])
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
        return E_SOK_SEL;
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

Socket::Socket() : mInit(false), mFd(-1)
{
}

Socket::~Socket()
{
    (void) this->close();
}

Result Socket::send(const Ipv4Address kDestIp,
                    const U16 kDestPort,
                    const void* const kBuf,
                    const U32 kNumBytes,
                    U32* const kNumBytesSent)
{
    // Check that socket is initialized.
    if (!mInit)
    {
        return E_SOK_UNINIT;
    }

    // Check that buffer is non-null.
    if (kBuf == nullptr)
    {
        return E_SOK_NULL;
    }

    // Create destination address.
    sockaddr_in destAddr;
    std::memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    const U32 ipNetOrder = ((static_cast<U32>(kDestIp.oct4) << 24) |
                            (static_cast<U32>(kDestIp.oct3) << 16) |
                            (static_cast<U32>(kDestIp.oct2) <<  8) |
                            (static_cast<U32>(kDestIp.oct1) <<  0));
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
        return E_SOK_SEND;
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
    // Check that socket is initialized.
    if (!mInit)
    {
        return E_SOK_UNINIT;
    }

    // Check that buffer is non-null.
    if (kBuf == nullptr)
    {
        return E_SOK_NULL;
    }

    // Receive into buffer.
    const I32 bytesRecvd = ::recv(mFd,
                                  const_cast<void*>(kBuf),
                                  kNumBytes,
                                  MSG_TRUNC);
    if (bytesRecvd < 0)
    {
        // Receive failed.
        return E_SOK_RECV;
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
    // Check that socket is initialized.
    if (!mInit)
    {
        return E_SOK_UNINIT;
    }

    // Close socket FD.
    if (::close(mFd) != 0)
    {
        return E_SOK_CLOSE;
    }

    // Reset socket FD and uninitialize socket.
    mFd = -1;
    mInit = false;

    return SUCCESS;
}
