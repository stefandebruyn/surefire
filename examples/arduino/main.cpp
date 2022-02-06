#include <Arduino.h>

#include "sfa/pal/Console.hpp"
#include "sfa/pal/Socket.hpp"

const IPv4Address gMyAddr = {10, 0, 0, 21};
const IPv4Address gTheirAddr = {10, 0, 0, 20};
const U16 gPort = 8000;
Socket gSock;

void setup()
{
    Serial.begin(9600);

    if (Socket::create(gMyAddr, 8000, Socket::UDP, gSock) != SUCCESS)
    {
        Console::printf("failed to create socket\n");
        while (true);
    }
}

void loop()
{
    if (gSock.send(gTheirAddr, gPort, "hello friend", 12, nullptr) != SUCCESS)
    {
        Console::printf("failed to send packet\n");
    }
    else
    {
        Console::printf("packet sent\n");
    }

    delay(1000);
}
