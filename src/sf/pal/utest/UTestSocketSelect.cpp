#include "sf/pal/Socket.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

// IPs used by test sockets.
static const IPv4Address gTestIp1 = {127, 0, 0, 1};
static const IPv4Address gTestIp2 = {127, 0, 0, 2};
static const IPv4Address gTestIp3 = {127, 0, 0, 3};
static const IPv4Address gTestIp4 = {127, 0, 0, 4};

// Port used by all test sockets.
static const U16 gTestPort = 7797;

// Test sockets.
static Socket gSock1;
static Socket gSock2;
static Socket gSock3;
static Socket gSock4;

/////////////////////////////////// Helpers ////////////////////////////////////

static void createSockets()
{
    CHECK_SUCCESS(Socket::init(gTestIp1, gTestPort, Socket::UDP, gSock1));
    CHECK_SUCCESS(Socket::init(gTestIp2, gTestPort, Socket::UDP, gSock2));
    CHECK_SUCCESS(Socket::init(gTestIp3, gTestPort, Socket::UDP, gSock3));
    CHECK_SUCCESS(Socket::init(gTestIp4, gTestPort, Socket::UDP, gSock4));
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(SocketSelect)
{
    void teardown()
    {
        // Close test sockets.
        gSock1.close();
        gSock2.close();
        gSock3.close();
        gSock4.close();
    }
};

TEST(SocketSelect, AllSocketsImmediatelyReady)
{
    createSockets();

    // Send messages from socket 4 to sockets 1, 2, and 3.
    const U64 msg1 = 1;
    const U64 msg2 = 2;
    const U64 msg3 = 3;
    CHECK_SUCCESS(gSock4.send(gTestIp1,
                              gTestPort,
                              &msg1,
                              sizeof(msg1),
                              nullptr));
    CHECK_SUCCESS(gSock4.send(gTestIp2,
                              gTestPort,
                              &msg2,
                              sizeof(msg2),
                              nullptr));
    CHECK_SUCCESS(gSock4.send(gTestIp3,
                              gTestPort,
                              &msg3,
                              sizeof(msg3),
                              nullptr));

    // Call `select` on sockets 1, 2, and 3.
    Socket* const socks[] = {&gSock1, &gSock2, &gSock3};
    bool ready[] = {false, false, false};
    U32 oldTimeoutUs = 1000;
    U32 timeoutUs = oldTimeoutUs;
    CHECK_SUCCESS(Socket::select(socks, ready, 3, timeoutUs));

    // All sockets are ready.
    CHECK_TRUE(ready[0]);
    CHECK_TRUE(ready[1]);
    CHECK_TRUE(ready[2]);

    // Some time elapsed but not the whole timeout.
    CHECK_TRUE(timeoutUs < oldTimeoutUs);
    CHECK_TRUE(timeoutUs > 0);

    // Read messages from sockets.
    U64 buf = 0;
    CHECK_SUCCESS(gSock1.recv(&buf, sizeof(buf), nullptr));
    CHECK_EQUAL(msg1, buf);
    CHECK_SUCCESS(gSock2.recv(&buf, sizeof(buf), nullptr));
    CHECK_EQUAL(msg2, buf);
    CHECK_SUCCESS(gSock3.recv(&buf, sizeof(buf), nullptr));
    CHECK_EQUAL(msg3, buf);
}

TEST(SocketSelect, SocketsReadyOneAtATime)
{
    createSockets();

    // Send message from socket 4 to socket 1.
    const U64 msg1 = 1;
    CHECK_SUCCESS(gSock4.send(gTestIp1,
                              gTestPort,
                              &msg1,
                              sizeof(msg1),
                              nullptr));

    // Call select on sockets 1, 2, and 3.
    Socket* const socks[] = {&gSock1, &gSock2, &gSock3};
    bool ready[] = {false, false, false};
    U32 oldTimeoutUs = 1000;
    U32 timeoutUs = oldTimeoutUs;
    CHECK_SUCCESS(Socket::select(socks, ready, 3, timeoutUs));

    // Only socket 1 is ready.
    CHECK_TRUE(ready[0]);
    CHECK_TRUE(!ready[1]);
    CHECK_TRUE(!ready[2]);

    // Some time elapsed but not the whole timeout.
    CHECK_TRUE(timeoutUs < oldTimeoutUs);
    CHECK_TRUE(timeoutUs > 0);

    // Read message from socket 1.
    U64 buf = 0;
    CHECK_SUCCESS(gSock1.recv(&buf, sizeof(buf), nullptr));
    CHECK_EQUAL(msg1, buf);

    // Send message from socket 4 to socket 2.
    const U64 msg2 = 2;
    CHECK_SUCCESS(gSock4.send(gTestIp2,
                              gTestPort,
                              &msg2,
                              sizeof(msg2),
                              nullptr));

    oldTimeoutUs = timeoutUs;
    CHECK_SUCCESS(Socket::select(socks, ready, 3, timeoutUs));

    // Socket 2 became ready. Socket 1 is still marked as ready from the
    // previous `select` call.
    CHECK_TRUE(ready[0]);
    CHECK_TRUE(ready[1]);
    CHECK_TRUE(!ready[2]);

    // More time elapsed but not the whole timeout.
    CHECK_TRUE(timeoutUs < oldTimeoutUs);
    CHECK_TRUE(timeoutUs > 0);

    // Read message from socket 2.
    CHECK_SUCCESS(gSock2.recv(&buf, sizeof(buf), nullptr));
    CHECK_EQUAL(msg2, buf);

    // Send message from socket 4 to socket 3.
    const U64 msg3 = 3;
    CHECK_SUCCESS(gSock4.send(gTestIp3,
                              gTestPort,
                              &msg3,
                              sizeof(msg3),
                              nullptr));

    oldTimeoutUs = timeoutUs;
    CHECK_SUCCESS(Socket::select(socks, ready, 3, timeoutUs));

    // Socket 3 became ready. Sockets 1 and 2 are still marked as ready from the
    // previous `select` calls.
    CHECK_TRUE(ready[0]);
    CHECK_TRUE(ready[1]);
    CHECK_TRUE(ready[2]);

    // More time elapsed but not the whole timeout.
    CHECK_TRUE(timeoutUs < oldTimeoutUs);
    CHECK_TRUE(timeoutUs > 0);

    // Read message from socket 3.
    CHECK_SUCCESS(gSock3.recv(&buf, sizeof(buf), nullptr));
    CHECK_EQUAL(msg3, buf);
}

TEST(SocketSelect, Timeout)
{
    createSockets();

    // Call select on sockets 1, 2, and 3 without sending them any messages.
    Socket* const socks[] = {&gSock1, &gSock2, &gSock3};
    bool ready[] = {false, false, false};
    U32 timeoutUs = 1000;
    CHECK_SUCCESS(Socket::select(socks, ready, 3, timeoutUs));

    // No sockets are ready.
    CHECK_TRUE(!ready[0]);
    CHECK_TRUE(!ready[1]);
    CHECK_TRUE(!ready[2]);

    // Timed out.
    CHECK_EQUAL(0, timeoutUs);
}

TEST(SocketSelect, ErrorUninitializedSocket)
{
    createSockets();
    Socket uninitSocket;
    Socket* const socks[] = {&gSock1, &gSock2, &uninitSocket};
    bool ready[] = {false, false, false};
    U32 timeoutUs = 1000;
    CHECK_ERROR(E_SOK_UNINIT, Socket::select(socks, ready, 3, timeoutUs));
}

TEST(SocketSelect, ErrorNullSocket)
{
    createSockets();
    Socket* const socks[] = {&gSock1, &gSock2, nullptr};
    bool ready[] = {false, false, false};
    U32 timeoutUs = 1000;
    CHECK_ERROR(E_SOK_NULL, Socket::select(socks, ready, 3, timeoutUs));
}

TEST(SocketSelect, ErrorNoSockets)
{
    createSockets();
    Socket* const socks[] = {&gSock1, &gSock2, &gSock3};
    bool ready[] = {false, false, false};
    U32 timeoutUs = 1000;
    CHECK_ERROR(E_SOK_SEL_NONE, Socket::select(socks, ready, 0, timeoutUs));
}
