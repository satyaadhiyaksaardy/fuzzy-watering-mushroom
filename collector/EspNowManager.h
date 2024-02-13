#ifndef ESPNOWMANAGER_H
#define ESPNOWMANAGER_H

#include <espnow.h>
#include "TelegramBotManager.h"
#include "Globals.h"

class EspNowManager
{
public:
    static void init()
    {
        if (esp_now_init() != 0)
        {
            Serial.println("Error initializing ESP-NOW");
            return;
        }
        esp_now_register_recv_cb(onDataReceive);
    }

private:
    static void onDataReceive(uint8_t *senderMac, uint8_t *incomingData, uint8_t len)
    {
        if (len == sizeof(struct_message))
        {
            struct_message msg;
            memcpy(&msg, incomingData, sizeof(msg));

            // Print the incoming data
            Serial.print("Received from Sensor ID: ");
            Serial.println(msg.id);
            Serial.print("Temperature: ");
            Serial.println(msg.temperature);
            Serial.print("Humidity: ");
            Serial.println(msg.humidity);

            if (msg.temperature == -999 || msg.humidity == -999) {
                errorInfo.hasError = true;
                errorInfo.sensorId = msg.id;
                errorInfo.lastErrorTime = millis();
            } else 
            {
                // Only accumulate data if both temperature and humidity are valid
                globalData.tempSum += msg.temperature;
                globalData.humiditySum += msg.humidity;
                globalData.count++;
            }

            // Calculate the average if count is sufficient
            if (globalData.count == numberOfSensors)
            { // Check if you want to calculate every time or after N samples
                globalData.averageTemperature = globalData.tempSum / globalData.count;
                globalData.averageHumidity = globalData.humiditySum / globalData.count;

                // Optionally, reset the sum and count after calculating the average
                globalData.tempSum = 0;
                globalData.humiditySum = 0;
                globalData.count = 0;
            }
        }
    }
};

#endif
