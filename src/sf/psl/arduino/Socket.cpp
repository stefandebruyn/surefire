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

#include <Ethernet.h>

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

    // Currently only supporting UDP.
    if (kProto != UDP)
    {
        return E_SOK_PROTO;
    }

    // Initialize Ethernet component on the first call to this function.
    static bool ethInit = false;
    if (!ethInit)
    {
        const byte macAddr[] =
        {
            ((SF_ARDUINO_MAC_ADDR >> 40) & 0xFF),
            ((SF_ARDUINO_MAC_ADDR >> 32) & 0xFF),
            ((SF_ARDUINO_MAC_ADDR >> 24) & 0xFF),
            ((SF_ARDUINO_MAC_ADDR >> 16) & 0xFF),
            ((SF_ARDUINO_MAC_ADDR >>  8) & 0xFF),
            ((SF_ARDUINO_MAC_ADDR >>  0) & 0xFF)
        };
        const IPAddress ipAddr(kIp.oct1, kIp.oct2, kIp.oct3, kIp.oct4);
        Ethernet.begin(macAddr, ipAddr);
        ethInit = true;
    }

    // Open socket.
    if (kSock.mUdp.begin(kPort) != 1)
    {
        return E_SOK_OPEN;
    }

    // Socket is ready - initialize it.
    kSock.mInit = true;

    return SUCCESS;
}

Socket::Socket() : mInit(false)
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

    // Address packet.
    IPAddress destIp(kDestIp.oct1, kDestIp.oct2, kDestIp.oct3, kDestIp.oct4);
    if (mUdp.beginPacket(destIp, kDestPort) != 1)
    {
        // "Problem resolving the hostname or port" according to Arduino docs.
        return E_SOK_SEND;
    }

    // Write packet payload.
    const U32 bytesSent = mUdp.write(static_cast<const char*>(kBuf), kNumBytes);

    // Return number of bytes sent if caller provided a pointer to do so.
    if (kNumBytesSent != nullptr)
    {
        *kNumBytesSent = bytesSent;
    }

    // Send packet.
    if (mUdp.endPacket() != 1)
    {
        // Unable to send packet.
        return E_SOK_SEND;
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

    // Wait for a packet to be available.
    while (mUdp.parsePacket() == 0);

    // Receive packet into buffer.
    const U32 bytesRecvd = mUdp.read(static_cast<char*>(kBuf), kNumBytes);

    // Return number of bytes received if caller provided a pointer to do so.
    if (kNumBytesRecvd != nullptr)
    {
        *kNumBytesRecvd = bytesRecvd;
    }

    return SUCCESS;
}

Result Socket::close()
{
    mUdp.stop();
    mInit = false;
    return SUCCESS;
}
