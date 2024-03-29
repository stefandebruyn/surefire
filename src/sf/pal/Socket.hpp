////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///                             ---------------
/// @file  sf/pal/Socket.hpp
/// @brief Platform-agnostic interface for network sockets.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_SOCKET_HPP
#define SF_SOCKET_HPP

#ifdef SF_PLATFORM_ARDUINO
#    include <EthernetUdp.h>
#endif

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Result.hpp"

namespace Sf
{

///
/// @brief Represents an IPv4 address.
///
struct Ipv4Address
{
    U8 oct1; ///< 1st octet
    U8 oct2; ///< 2nd octet
    U8 oct3; ///< 3rd octet
    U8 oct4; ///< 4th octet
};

///
/// @brief Platform-agnostic handle to a network socket.
///
/// Socket uses the same factory method and RAII patterns as most objects in the
/// framework. The user default-constructs a Socket and then passes it to a
/// factory method that initializes it. The "resource" represented by a Socket
/// is the socket object in the underlying platform. The socket is tied to
/// the lifetime of the Socket and is closed when the Socket destructs.
///
class Socket final
{
public:

    ///
    /// @brief Possible socket communication protocols.
    ///
    enum Protocol : U8
    {
        UDP = 0 ///< UDP/IP
    };

    ///
    /// @brief Initializes a socket.
    ///
    /// @pre  kSock is uninitialized.
    /// @post On success, kSock is bound to the specified address and ready to
    ///       perform network functions. Invoking methods on it may succeed.
    /// @post On error, preconditions still hold.
    ///
    /// @param[in] kIp     Socket IP address.
    /// @param[in] kPort   Socket port.
    /// @param[in] kProto  Socket protocol.
    /// @param[in] kSock   Socket to initialize.
    ///
    /// @retval SUCCESS       Successfully initialized.
    /// @retval E_SOK_REINIT  kSock is already initialized.
    /// @retval E_SOK_PROTO   kProto is unsupported.
    /// @retval E_SOK_OPEN    Failed to open socket.
    /// @retval E_SOK_BIND    Failed to bind socket to address.
    ///
    static Result init(const Ipv4Address kIp,
                       const U16 kPort,
                       const Protocol kProto,
                       Socket& kSock);

    ///
    /// @brief Waits for multiple sockets to have data available with a timeout,
    /// returning once at least one socket has data avilable or the timeout
    /// expires.
    ///
    /// @remark Linux: The Linux PSL implementation of this function uses the
    /// select() system call, which returns to the caller how much of the
    /// timeout was not used. This is not required by POSIX, so if the Linux PSL
    /// was compiled for another POSIX platform, Thread::select() might not
    /// update the timeout value passed to it. This is one of only a few known
    /// potential incompatibilities between the Linux PSL and other POSIX
    /// platforms.
    ///
    /// @param[in]      kSocks      Array of sockets to wait on.
    /// @param[in, out] kReady      Array of socket ready flags. kSocks[i] is
    ///                             eligible for selection only if kReady[i] is
    ///                             false. On success and kSocks[i] has data
    ///                             available, kReady[i] is set to true.
    /// @param[in]      kNumSocks   Size of kSocks and kReady arrays. This must
    ///                             be at least 1.
    /// @param[in, out] kTimeoutUs  Timeout in microseconds. A timeout of 0 will
    ///                             poll. On success, the reference is updated
    ///                             to store how much time was not used, where 0
    ///                             indicates a timeout. It is technically
    ///                             possible for the timeout to exactly elapse
    ///                             as a socket becomes available to read, so
    ///                             the caller should check kReady for a ready
    ///                             socket even when kTimeoutUs is 0.
    ///
    /// @retval SUCCESS         Select successful. This does not necessarily
    ///                         mean a socket became available for reading.
    /// @retval E_SOK_NULL      kSocks is null, kReady is null, or kSocks
    ///                         contains a null pointer.
    /// @retval E_SOK_SEL_NONE  kSocks is empty.
    /// @retval E_SOK_UNINIT    kSocks contains an uninitialized socket.
    /// @retval E_SOK_SEL       Linux: select() system call failed.
    ///
    static Result select(Socket* const kSocks[],
                         bool* const kReady,
                         const U32 kNumSocks,
                         U32& kTimeoutUs);

    ///
    /// @brief Default constructor.
    ///
    /// @post The constructed Socket is uninitialized and invoking any of its
    /// methods returns an error.
    ///
    Socket();

    ///
    /// @brief Destructor.
    ///
    /// @post If the Socket was initialized, the underlying socket was closed.
    ///
    ~Socket();

    ///
    /// @brief Sends data to an address.
    ///
    /// @param[in]  kDestIp        Destination IP address.
    /// @param[in]  kDestPort      Destination port.
    /// @param[in]  kBuf           Pointer to data to send.
    /// @param[in]  kNumBytes      Size of data to send in bytes.
    /// @param[out] kNumBytesSent  On success, if non-null, will be set to the
    ///                            number of bytes sent. It is not an error if
    ///                            this does not equal kNumBytes.
    ///
    /// @retval SUCCESS       Successfully sent data.
    /// @retval E_SOK_UNINIT  Socket is uninitialized.
    /// @retval E_SOK_NULL    kBuf is null.
    /// @retval E_SOK_SEND    Send failed.
    ///
    Result send(const Ipv4Address kDestIp,
                const U16 kDestPort,
                const void* const kBuf,
                const U32 kNumBytes,
                U32* const kNumBytesSent);

    ///
    /// @brief Receives data addressed to the socket from any address, blocking
    /// until something is received.
    ///
    /// @param[out] kBuf            Pointer to buffer to read into.
    /// @param[in]  kNumBytes       Number of bytes to read. The buffer must be
    ///                             at least this size.
    /// @param[out] kNumBytesRecvd  On success, if non-null, will be set to the
    ///                             number of bytes received. It is not an error
    ///                             if this does not equal kNumBytes.
    ///
    /// @retval SUCCESS       Successfully received data.
    /// @retval E_SOK_UNINIT  Socket is uninitialized.
    /// @retval E_SOK_NULL    kBuf is null.
    /// @retval E_SOK_RECV    Receive failed.
    ///
    Result recv(void* const kBuf,
                const U32 kNumBytes,
                U32* const kNumBytesRecvd);

    ///
    /// @brief Closes the socket, releasing any acquired resources. The Socket
    /// may be initialized again afterwards.
    ///
    /// @retval SUCCESS       Successfully closed.
    /// @retval E_SOK_UNINIT  Socket is uninitialized.
    ///
    Result close();

    Socket(const Socket&) = delete;
    Socket(Socket&&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket& operator=(Socket&&) = delete;

private:

    ///
    /// @brief Whether the socket is initialized.
    ///
    bool mInit;

#ifdef SF_PLATFORM_LINUX

    ///
    /// @brief Socket file descriptor.
    ///
    I32 mFd;

#elif defined(SF_PLATFORM_ARDUINO)

    ///
    /// @brief UDP object.
    ///
    EthernetUDP mUdp;

#endif
};

} // namespace Sf

#endif
