#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#include "sfa/core/BasicTypes.hpp"
#include "sfa/core/Element.hpp"

U8 mac[] = {0xa8, 0x61, 0x0a, 0xae, 0x75, 0x9c};
IPAddress ip(10, 0, 0, 21);
U16 port = 8000;
EthernetUDP udp;

I32 y;
Element<I32> x(y);

void setup()
{
    x.write(100);
    Serial.begin(9600);
    Ethernet.begin(mac, ip);
    udp.begin(port);
}

void loop()
{
    delay(1000);
    if (Ethernet.linkStatus() == LinkON)
    {
        udp.beginPacket(IPAddress(10, 0, 0, 20), port);
        udp.write("hello friend", 12);
        udp.endPacket();
        Serial.println("sent packet");
    }
    else
    {
        Serial.println("link off");
    }
}
