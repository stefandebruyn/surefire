#ifndef SFA_SOCKET_HPP
#define SFA_SOCKET_HPP

#ifdef SFA_PLATFORM_ARDUINO
#    include <EthernetUdp.h>
#endif

#include "sfa/pal/Platform.hpp"
#include "sfa/core/Result.hpp"
#include "sfa/core/BasicTypes.hpp"

struct IPv4Address
{
    U32 oct1;
    U32 oct2;
    U32 oct3;
    U32 oct4;
};

class Socket final
{
public:

    enum Protocol : U8
    {
        UDP
    };

    static Result create(const IPv4Address kIp,
                         const U16 kPort,
                         const Protocol kProto,
                         Socket& kSock);

    static Result select(Socket* const kSocks[],
                         bool* const kReady,
                         const U32 kNumSocks,
                         U32& kTimeoutUs);

    Socket();

    Result send(const IPv4Address kDestIp,
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
#elif defined(SFA_PLATFORM_ARDUINO)
    bool mInit;

    EthernetUDP mUdp;
#endif
};

#endif
