#include <Arduino.h>

#include "sf/core/Diagnostic.hpp"
#include "sf/pal/Clock.hpp"
#include "sf/pal/Console.hpp"
#include "sf/pal/Socket.hpp"
#include "sf/pal/DigitalIO.hpp"

const IPv4Address gMyAddr = {10, 0, 0, 21};
const IPv4Address gTheirAddr = {10, 0, 0, 20};
const U16 gPort = 8080;
Socket gSock;
DigitalIO gDio;

void setup()
{
    // Initialize stuff.
    Serial.begin(9600);
    Diag::haltOnError(Socket::init(gMyAddr, gPort, Socket::UDP, gSock),
                      "Failed to create socket");
    Diag::haltOnError(DigitalIO::init(gDio), "Failed to initialize DIO");
    Diag::haltOnError(gDio.setMode(LED_BUILTIN, DigitalIO::OUT),
                      "failed to set DIO pin as output");
}

void loop()
{
    const U32 seconds = (Clock::nanoTime() / Clock::NS_IN_S);
    Console::printf("The time is %u seconds. Sending message...\n", seconds);

    // Send a message.
    Diag::printOnError(
        gSock.send(gTheirAddr, gPort, "hello friend", 12, nullptr),
        "Failed to send message");

    // Blink the LED.
    static bool ledOn = false;
    Diag::printOnError(gDio.write(LED_BUILTIN, ledOn),
                       "Failed to write DIO pin");
    ledOn = !ledOn;

    // Take a nap.
    delay(1000);
}
