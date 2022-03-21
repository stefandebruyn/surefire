#ifndef SF_SOCKET_HPP
#define SF_SOCKET_HPP

#ifdef SF_PLATFORM_ARDUINO
#    include <EthernetUdp.h>
#endif

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Result.hpp"

struct IPv4Address
{
    U8 oct1;
    U8 oct2;
    U8 oct3;
    U8 oct4;
};

class Socket final
{
public:

    enum Protocol : U8
    {
        UDP = 0
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

    ~Socket();

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

#ifdef SF_PLATFORM_LINUX
    I32 mFd;
#elif defined(SF_PLATFORM_ARDUINO)
    bool mInit;

    EthernetUDP mUdp;
#endif
};

#endif
