#ifndef SFA_SOCKET_HPP
#define SFA_SOCKET_HPP

#include "sfa/Result.hpp"
#include "sfa/BasicTypes.hpp"

namespace Socket
{
    enum Protocol : U8
    {
        UDP
    };

    Result create(const char* const kIp,
                  const U16 kPort,
                  const Protocol kProto,
                  I32& kSock);

    Result send(const I32 kSock,
                const char* const kDestIp,
                const U16 kDestPort,
                const void* const kBuf,
                const U32 kNumBytes,
                U32* const kNumBytesSent);

    Result recv(const I32 kSock,
                void* const kBuf,
                const U32 kNumBytes,
                U32* const kNumBytesRecvd);

    Result select(const I32* const kSocks,
                  bool* const kReady,
                  const U32 kNumSocks,
                  U32& kTimeoutUs);

    Result close(const I32 kSock);
}

#endif
