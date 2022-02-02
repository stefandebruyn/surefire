#include "pal/Socket.hpp"
#include "UTest.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

// IPs used for test sockets.
#define TEST_IP1 ("127.0.0.1")
#define TEST_IP2 ("127.0.0.2")

// Port used by all test sockets.
#define TEST_PORT (7797)

// Test socket descriptors.
static I32 gSock1;
static I32 gSock2;

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(UdpSocket)
{
    void setup()
    {
        // Uninitialize test socket descriptors.
        gSock1 = -1;
        gSock2 = -1;
    }

    void teardown()
    {
        // Close test sockets.
        Socket::close(gSock1);
        Socket::close(gSock2);
    }
};

TEST(UdpSocket, CreateAndClose)
{
    CHECK_SUCCESS(Socket::create(TEST_IP1, TEST_PORT, Socket::UDP, gSock1));
    CHECK_TRUE(gSock1 >= 0);
    CHECK_SUCCESS(Socket::close(gSock1));
}

TEST(UdpSocket, CreateInvalidProtocol)
{
    CHECK_ERROR(E_SOCK_PROTO,
                Socket::create(TEST_IP1,
                               TEST_PORT,
                               static_cast<Socket::Protocol>(0xFF),
                               gSock1));
    CHECK_EQUAL(-1, gSock1);
}

TEST(UdpSocket, CreateInvalidIp)
{
    CHECK_ERROR(E_SOCK_BIND,
                Socket::create("123.123.123.123",
                               TEST_PORT,
                               Socket::UDP,
                               gSock1));
    CHECK_EQUAL(-1, gSock1);
}

TEST(UdpSocket, CreateNullIp)
{
    CHECK_ERROR(E_SOCK_NULL,
                Socket::create(nullptr,
                               TEST_PORT,
                               Socket::UDP,
                               gSock1));
    CHECK_EQUAL(-1, gSock1);
}

TEST(UdpSocket, CreatePortInUse)
{
    CHECK_SUCCESS(Socket::create(TEST_IP1, TEST_PORT, Socket::UDP, gSock1));
    CHECK_ERROR(E_SOCK_BIND,
                Socket::create(TEST_IP1,
                               TEST_PORT,
                               Socket::UDP,
                               gSock2));
    CHECK_EQUAL(-1, gSock2);
}

TEST(UdpSocket, SmallSendAndRecv)
{
    // Open sockets.
    CHECK_SUCCESS(Socket::create(TEST_IP1, TEST_PORT, Socket::UDP, gSock1));
    CHECK_SUCCESS(Socket::create(TEST_IP2, TEST_PORT, Socket::UDP, gSock2));

    // Small 11-byte message.
    U8 buf1[] =
        {0xFF, 0xA1, 0x07, 0x00, 0x8D, 0x12, 0xF4, 0x44, 0x90, 0x35, 0x26};
    U8 buf2[sizeof(buf1)] = {};

    // Send `buf1` from socket 1 to socket 2.
    U32 bytesSent = 0;
    CHECK_SUCCESS(Socket::send(gSock1,
                               TEST_IP2,
                               TEST_PORT,
                               buf1,
                               sizeof(buf1),
                               &bytesSent));
    CHECK_EQUAL(sizeof(buf1), bytesSent);

    // Receive from socket 2 into `buf2`.
    U32 bytesRecvd = 0;
    CHECK_SUCCESS(Socket::recv(gSock2, buf2, sizeof(buf2), &bytesRecvd));
    CHECK_EQUAL(sizeof(buf2), bytesRecvd);

    // Buffers are equal.
    MEMCMP_EQUAL(buf1, buf2, sizeof(buf1));
}

TEST(UdpSocket, LargeSendAndRecv)
{
    // Open sockets.
    CHECK_SUCCESS(Socket::create(TEST_IP1, TEST_PORT, Socket::UDP, gSock1));
    CHECK_SUCCESS(Socket::create(TEST_IP2, TEST_PORT, Socket::UDP, gSock2));

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
    CHECK_SUCCESS(Socket::send(gSock1,
                               TEST_IP2,
                               TEST_PORT,
                               buf1,
                               sizeof(buf1),
                               &bytesSent));
    CHECK_EQUAL(sizeof(buf1), bytesSent);

    // Receive from socket 2 into `buf2`.
    U32 bytesRecvd = 0;
    CHECK_SUCCESS(Socket::recv(gSock2, buf2, sizeof(buf2), &bytesRecvd));
    CHECK_EQUAL(sizeof(buf2), bytesRecvd);

    // Buffers are equal.
    MEMCMP_EQUAL(buf1, buf2, sizeof(buf1));
}

TEST(UdpSocket, SendAndRecvNullNumBytesPtr)
{
    // Open sockets.
    CHECK_SUCCESS(Socket::create(TEST_IP1, TEST_PORT, Socket::UDP, gSock1));
    CHECK_SUCCESS(Socket::create(TEST_IP2, TEST_PORT, Socket::UDP, gSock2));

    // Small 11-byte message.
    U8 buf1[] =
        {0xFF, 0xA1, 0x07, 0x00, 0x8D, 0x12, 0xF4, 0x44, 0x90, 0x35, 0x26};
    U8 buf2[sizeof(buf1)] = {};

    // Send `buf1` from socket 1 to socket 2.
    CHECK_SUCCESS(Socket::send(gSock1,
                               TEST_IP2,
                               TEST_PORT,
                               buf1,
                               sizeof(buf1),
                               nullptr));

    // Receive from socket 2 into `buf2`.
    CHECK_SUCCESS(Socket::recv(gSock2, buf2, sizeof(buf2), nullptr));

    // Buffers are equal.
    MEMCMP_EQUAL(buf1, buf2, sizeof(buf1));
}

TEST(UdpSocket, SendNullIp)
{
    CHECK_SUCCESS(Socket::create(TEST_IP1, TEST_PORT, Socket::UDP, gSock1));
    U64 buf;
    CHECK_ERROR(E_SOCK_NULL,
                Socket::send(gSock1,
                             nullptr,
                             TEST_PORT,
                             &buf,
                             sizeof(buf),
                             nullptr));
}

TEST(UdpSocket, SendNullBuffer)
{
    CHECK_SUCCESS(Socket::create(TEST_IP1, TEST_PORT, Socket::UDP, gSock1));
    CHECK_ERROR(E_SOCK_NULL,
                Socket::send(gSock1,
                             TEST_IP2,
                             TEST_PORT,
                             nullptr,
                             8,
                             nullptr));
}

TEST(UdpSocket, SendInvalidSocket)
{
    U64 buf;
    CHECK_ERROR(E_SOCK_SEND,
                Socket::send(-1,
                             TEST_IP1,
                             TEST_PORT,
                             &buf,
                             sizeof(buf),
                             nullptr));
}

TEST(UdpSocket, RecvNullBuffer)
{
    CHECK_SUCCESS(Socket::create(TEST_IP1, TEST_PORT, Socket::UDP, gSock1));
    CHECK_ERROR(E_SOCK_NULL, Socket::recv(gSock1, nullptr, 8, nullptr));
}

TEST(UdpSocket, RecvInvalidSocket)
{
    U64 buf;
    CHECK_ERROR(E_SOCK_RECV, Socket::recv(-1, &buf, sizeof(buf), nullptr));
}

TEST(UdpSocket, CloseError)
{
    CHECK_ERROR(E_SOCK_CLOSE, Socket::close(-1));
}
