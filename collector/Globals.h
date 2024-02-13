#ifndef GLOBALS_H
#define GLOBALS_H

#include <ESP8266TelegramBOT.h>
#include "SensorData.h" // Assuming struct_message is defined here

// Define a global structure for shared data
struct GlobalData
{
    float tempSum = 0;
    float humiditySum = 0;
    int count = 0;
    float averageTemperature = 0;
    float averageHumidity = 0;

    float wateringDuration = 0;
    bool autoWateringEnabled = false;
    unsigned long lastAutoWateringTime = 0;
    bool isWatering = false;
    unsigned long wateringStartTime = 0;
};

extern GlobalData globalData; // Declare the global variable

struct ErrorInfo
{
    bool hasError = false;
    int sensorId = -1;
    unsigned long lastErrorTime = 0;
    const unsigned long errorTimeInterval = 600000;
};

extern ErrorInfo errorInfo;

// Sensor setup
extern const int numberOfSensors;

// Network credentials
extern const char *ssid;
extern const char *password;

// Telegram bot setup
extern TelegramBOT bot;
extern const char *botToken;
extern const char *botName;
extern const char *botUsername;
extern String botCommand;
extern String botResponse;
extern String sender;
extern String chatId;
extern unsigned long botScanInterval;
extern unsigned long botLastScan;
extern unsigned long nowMillis;

// Bot Owner ID
extern String users[];
extern const int userCount;

// Actuator setup
extern const int pumpPin;
extern const int fanPin;

#endif // GLOBALS_H
