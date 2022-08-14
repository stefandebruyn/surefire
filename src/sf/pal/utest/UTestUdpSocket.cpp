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
///
///                             ---------------
/// @file  sf/core/utest/UTestUdpSocket.cpp
/// @brief Unit tests for UDP sockets.
////////////////////////////////////////////////////////////////////////////////

#include "sf/pal/Socket.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Global /////////////////////////////////////

// IPs used for test sockets.
static const Ipv4Address gTestIp1 = {127, 0, 0, 1};
static const Ipv4Address gTestIp2 = {127, 0, 0, 2};

// Port used by all test sockets.
static const U16 gTestPort = 7797;

// Test sockets.
static Socket gSock1;
static Socket gSock2;

///
/// @brief Checks that invoking all methods on a socket fail with E_SOK_UNINIT.
///
/// @param[in] kSock  Uninitialized socket.
///
inline void checkSocketUninitialized(Socket& kSock)
{
    U64 buf;
    CHECK_ERROR(E_SOK_UNINIT,
                kSock.send(gTestIp1, gTestPort, &buf, sizeof(buf), nullptr));
    CHECK_ERROR(E_SOK_UNINIT, kSock.recv(&buf, sizeof(buf), nullptr));
    CHECK_ERROR(E_SOK_UNINIT, kSock.close());
}

//////////////////////////////////// Tests /////////////////////////////////////

///
/// @brief Unit tests for UDP sockets.
///
TEST_GROUP(UdpSocket)
{
    void setup()
    {
    }

    void teardown()
    {
        // Close test sockets.
        (void) gSock1.close();
        (void) gSock2.close();
    }
};

///
/// @test Socket is uninitialized by default.
///
TEST(UdpSocket, Uninitialized)
{
    Socket socket;
    checkSocketUninitialized(socket);
}

///
/// @test Socket can be initialized, used to send data, and closed.
///
TEST(UdpSocket, CreateSendAndClose)
{
    CHECK_SUCCESS(Socket::init(gTestIp1, gTestPort, Socket::UDP, gSock1));
    U64 buf = 0;
    CHECK_SUCCESS(gSock1.send(gTestIp2, gTestPort, &buf, sizeof(buf), nullptr));
    CHECK_SUCCESS(gSock1.close());
    checkSocketUninitialized(gSock1);
}

///
/// @test Socket can be reused after closing.
///
TEST(UdpSocket, CreateReuse)
{
    CHECK_SUCCESS(Socket::init(gTestIp1, gTestPort, Socket::UDP, gSock1));
    CHECK_SUCCESS(gSock1.close());
    CHECK_SUCCESS(Socket::init(gTestIp1, gTestPort, Socket::UDP, gSock1));
}

///
/// @test Creating a socket with an invalid protocol returns an error.
///
TEST(UdpSocket, ErrorCreateInvalidProtocol)
{
    CHECK_ERROR(E_SOK_PROTO,
                Socket::init(gTestIp1,
                               gTestPort,
                               static_cast<Socket::Protocol>(0xFF),
                               gSock1));
    checkSocketUninitialized(gSock1);
}

///
/// @test Creating a socket with an IP to which it cannot bind returns an error.
///
TEST(UdpSocket, ErrorCreateInvalidIp)
{
    const Ipv4Address invalidIp = {123, 123, 123, 123};
    CHECK_ERROR(E_SOK_BIND,
                Socket::init(invalidIp, gTestPort, Socket::UDP, gSock1));
    checkSocketUninitialized(gSock1);
}

///
/// @test Creating a socket on a busy port returns an error.
///
TEST(UdpSocket, ErrorCreatePortInUse)
{
    CHECK_SUCCESS(Socket::init(gTestIp1, gTestPort, Socket::UDP, gSock1));
    CHECK_ERROR(E_SOK_BIND,
                Socket::init(gTestIp1, gTestPort, Socket::UDP, gSock2));
    checkSocketUninitialized(gSock2);
}

///
/// @test Initializing a socket twice returns an error.
///
TEST(UdpSocket, ErrorCreateReinitialize)
{
    CHECK_SUCCESS(Socket::init(gTestIp1, gTestPort, Socket::UDP, gSock1));
    CHECK_ERROR(E_SOK_REINIT,
                Socket::init(gTestIp2, gTestPort, Socket::UDP, gSock1));
}

///
/// @test Initialized socket is closed on destruction.
///
TEST(UdpSocket, DestructInitialized)
{
    {
        Socket sock;
        CHECK_SUCCESS(Socket::init(gTestIp1, gTestPort, Socket::UDP, sock));
    }
    CHECK_SUCCESS(Socket::init(gTestIp1, gTestPort, Socket::UDP, gSock1));
}

///
/// @test Destructing an uninitialized socket does nothing.
///
TEST(UdpSocket, DestructUninitialized)
{
    {
        Socket sock;
    }
}

///
/// @test Small amounts of data are sent and received successfully.
///
TEST(UdpSocket, SmallSendAndRecv)
{
    // Open sockets.
    CHECK_SUCCESS(Socket::init(gTestIp1, gTestPort, Socket::UDP, gSock1));
    CHECK_SUCCESS(Socket::init(gTestIp2, gTestPort, Socket::UDP, gSock2));

    // Small 11-byte message.
    U8 buf1[] =
        {0xFF, 0xA1, 0x07, 0x00, 0x8D, 0x12, 0xF4, 0x44, 0x90, 0x35, 0x26};
    U8 buf2[sizeof(buf1)] = {};

    // Send `buf1` from socket 1 to socket 2.
    U32 bytesSent = 0;
    CHECK_SUCCESS(gSock1.send(gTestIp2,
                              gTestPort,
                              buf1,
                              sizeof(buf1),
                              &bytesSent));
    CHECK_EQUAL(sizeof(buf1), bytesSent);

    // Receive from socket 2 into `buf2`.
    U32 bytesRecvd = 0;
    CHECK_SUCCESS(gSock2.recv(buf2, sizeof(buf2), &bytesRecvd));
    CHECK_EQUAL(sizeof(buf2), bytesRecvd);

    // Buffers are equal.
    MEMCMP_EQUAL(buf1, buf2, sizeof(buf1));
}

///
/// @test Large (relatively) amounts of data are sent and received successfully.
///
TEST(UdpSocket, LargeSendAndRecv)
{
    // Open sockets.
    CHECK_SUCCESS(Socket::init(gTestIp1, gTestPort, Socket::UDP, gSock1));
    CHECK_SUCCESS(Socket::init(gTestIp2, gTestPort, Socket::UDP, gSock2));

    // 4-kilobyte message.
    U8 buf1[4096];
    U8 buf2[sizeof(buf1)] = {};

    // Fill the message with various numbers.
    for (U32 i = 0; i < sizeof(buf1); ++i)
    {
        buf1[i] = (i % 256);
    }

    // Send `buf1` from socket 1 to socket 2.
    U32 bytesSent = 0;
    CHECK_SUCCESS(gSock1.send(gTestIp2,
                              gTestPort,
                              buf1,
                              sizeof(buf1),
                              &bytesSent));
    CHECK_EQUAL(sizeof(buf1), bytesSent);

    // Receive from socket 2 into `buf2`.
    U32 bytesRecvd = 0;
    CHECK_SUCCESS(gSock2.recv(buf2, sizeof(buf2), &bytesRecvd));
    CHECK_EQUAL(sizeof(buf2), bytesRecvd);

    // Buffers are equal.
    MEMCMP_EQUAL(buf1, buf2, sizeof(buf1));
}

///
/// @test A null num bytes pointer passed to Socket::send() is not dereferenced.
///
TEST(UdpSocket, SendAndRecvNullNumBytesPtr)
{
    // Open sockets.
    CHECK_SUCCESS(Socket::init(gTestIp1, gTestPort, Socket::UDP, gSock1));
    CHECK_SUCCESS(Socket::init(gTestIp2, gTestPort, Socket::UDP, gSock2));

    // Small 11-byte message.
    U8 buf1[] =
        {0xFF, 0xA1, 0x07, 0x00, 0x8D, 0x12, 0xF4, 0x44, 0x90, 0x35, 0x26};
    U8 buf2[sizeof(buf1)] = {};

    // Send `buf1` from socket 1 to socket 2.
    CHECK_SUCCESS(gSock1.send(gTestIp2,
                              gTestPort,
                              buf1,
                              sizeof(buf1),
                              nullptr));

    // Receive from socket 2 into `buf2`.
    CHECK_SUCCESS(gSock2.recv(buf2, sizeof(buf2), nullptr));

    // Buffers are equal.
    MEMCMP_EQUAL(buf1, buf2, sizeof(buf1));
}

///
/// @test Sending a null buffer returns an error.
///
TEST(UdpSocket, ErrorSendNullBuffer)
{
    CHECK_SUCCESS(Socket::init(gTestIp1, gTestPort, Socket::UDP, gSock1));
    CHECK_ERROR(E_SOK_NULL, gSock1.send(gTestIp2,
                                         gTestPort,
                                         nullptr,
                                         8,
                                         nullptr));
}

///
/// @test Receiving into a null buffer returns an error.
///
TEST(UdpSocket, ErrorRecvNullBuffer)
{
    CHECK_SUCCESS(Socket::init(gTestIp1, gTestPort, Socket::UDP, gSock1));
    CHECK_ERROR(E_SOK_NULL, gSock1.recv(nullptr, 8, nullptr));
}
