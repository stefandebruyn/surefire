#ifndef SFA_SOCKET_HPP
#define SFA_SOCKET_HPP

#include "sfa/pal/Platform.hpp"
#include "sfa/Result.hpp"
#include "sfa/BasicTypes.hpp"

class Socket final
{
public:

    enum Protocol : U8
    {
        UDP
    };

    static Result create(const char* const kIp,
                         const U16 kPort,
                         const Protocol kProto,
                         Socket& kSock);

    static Result select(Socket* const kSocks[],
                         bool* const kReady,
                         const U32 kNumSocks,
                         U32& kTimeoutUs);

    Socket();

    Result send(const char* const kDestIp,
                const U16 kDestPort,
                const void* const kBuf,
                const U32 kNumBytes,
                U32* const kNumBytesSent);

    Result recv(void* const kBuf,
                const U32 kNumBytes,
                U32* const kNumBytesRecvd);

    Result close();

    Socket(const Socket&) = delete;
    Socket(Socket&&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket& operator=(Socket&&) = delete;

private:

#ifdef SFA_PLATFORM_LINUX
    I32 mFd;
#endif
};

#endif
