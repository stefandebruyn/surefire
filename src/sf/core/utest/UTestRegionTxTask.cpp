////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
///
///                             ---------------
/// @file  sf/core/utest/UTestRegionTxTask.hpp
/// @brief Unit tests for RegionTxTask.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/RegionTxTask.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

// IPs and ports used by test sockets.
static const IPv4Address gTaskIp = {127, 0, 0, 1};
static const IPv4Address gRecvIp = {127, 0, 0, 2};
static const U16 gPort = 7797;

//////////////////////////////////// Tests /////////////////////////////////////

///
/// @brief Unit tests for RegionTxTask.
///
TEST_GROUP(RegionTxTask)
{
};

///
/// @test Task sends region once per step when enabled.
///
TEST(RegionTxTask, SendRegion)
{
    // Create a region with some arbitrary data.
    U32 data = 0xDEADBEEF;
    Region region(&data, sizeof(data));

    // Initialize socket for task to use.
    Socket taskSock;
    CHECK_SUCCESS(Socket::init(gTaskIp, gPort, Socket::UDP, taskSock));

    // Initialize socket to receive region on.
    Socket recvSock;
    CHECK_SUCCESS(Socket::init(gRecvIp, gPort, Socket::UDP, recvSock));

    // Initialize task and step it once. Expect it to emit a region.
    RegionTxTask task(nullptr, {taskSock, region, gRecvIp, gPort, nullptr});
    CHECK_SUCCESS(task.init());
    CHECK_SUCCESS(task.step());

    // Receive region sent by task.
    U32 recvData = 0;
    CHECK_SUCCESS(recvSock.recv(&recvData, sizeof(data), nullptr));
    CHECK_EQUAL(data, recvData);

    // Polling the socket yields nothing since only 1 region was sent.
    Socket* const sockets[] = {&recvSock};
    bool ready = false;
    U32 timeout = 0;
    CHECK_SUCCESS(Socket::select(sockets, &ready, 1, timeout));
    CHECK_TRUE(!ready);
}

///
/// @test Task does nothing when safed.
///
TEST(RegionTxTask, SafeModeNop)
{
    // Create a region with some arbitrary data.
    U32 data = 0xDEADBEEF;
    Region region(&data, sizeof(data));

    // Initialize socket for task to use.
    Socket taskSock;
    CHECK_SUCCESS(Socket::init(gTaskIp, gPort, Socket::UDP, taskSock));

    // Initialize socket to receive region on.
    Socket recvSock;
    CHECK_SUCCESS(Socket::init(gRecvIp, gPort, Socket::UDP, recvSock));

    // Initialize task and step it once in safe mode.
    U8 mode = TaskMode::SAFE;
    Element<U8> elemMode(mode);
    RegionTxTask task(&elemMode, {taskSock, region, gRecvIp, gPort, nullptr});
    CHECK_SUCCESS(task.init());
    CHECK_SUCCESS(task.step());

    // No region was received since task stepped in safe mode.
    Socket* const sockets[] = {&recvSock};
    bool ready = false;
    U32 timeout = 0;
    CHECK_SUCCESS(Socket::select(sockets, &ready, 1, timeout));
    CHECK_TRUE(!ready);
}

///
/// @test Task increments the tx count element on successfuly send if the
/// element is provided.
///
TEST(RegionTxTask, IncrementTxCountElement)
{
    // Create a tx count element.
    U32 txCnt = 0;
    Element<U32> elemTxCnt(txCnt);

    // Create a region with some arbitrary data.
    U32 data = 0xDEADBEEF;
    Region region(&data, sizeof(data));

    // Initialize socket for task to use.
    Socket taskSock;
    CHECK_SUCCESS(Socket::init(gTaskIp, gPort, Socket::UDP, taskSock));

    // Initialize task and step it thrice.
    RegionTxTask task(nullptr, {taskSock, region, gRecvIp, gPort, &elemTxCnt});
    CHECK_SUCCESS(task.init());

    CHECK_SUCCESS(task.step());
    CHECK_EQUAL(1, elemTxCnt.read());
    CHECK_SUCCESS(task.step());
    CHECK_EQUAL(2, elemTxCnt.read());
    CHECK_SUCCESS(task.step());
    CHECK_EQUAL(3, elemTxCnt.read());
}
