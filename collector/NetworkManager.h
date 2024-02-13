#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <ESP8266WiFi.h>

class NetworkManager
{
public:
    static void init(const char *ssid, const char *password)
    {
        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(ssid, password);
        Serial.printf("Connecting to %s .", ssid);
        while (WiFi.status() != WL_CONNECTED)
        {
            Serial.print(".");
            delay(200);
        }
        Serial.println("Connected.");
        Serial.printf("IP address: %s\n", WiFi.localIP());
        Serial.printf("SSID: %s\n", ssid);
        Serial.printf("Channel: %u\n", WiFi.channel());
        delay(1000);
    }
};

#endif
