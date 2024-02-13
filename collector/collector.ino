#include <ESP8266WiFi.h>
#include "NetworkManager.h"
#include "EspNowManager.h"
#include "TelegramBotManager.h"
#include "SensorData.h"
#include "FuzzyLogicController.h"
#include "ActuatorControl.h"
#include "Globals.h"

GlobalData globalData;
ErrorInfo errorInfo;

// Define your network credentials
const char *ssid = "YOUR_SSID";
const char *password = "YOUR_PASSWORD";

// Define your actuator pins
const int pumpPin = 5;
const int fanPin = 4;

// Define your sensor number
const int numberOfSensors = 2;

// Define your Telegram bot settings
const char *botToken = "YOUR_BOT_TOKEN";
const char *botName = "YOUR_BOT_NAME";
const char *botUsername = "YOUR_BOT_USERNAME";
TelegramBOT bot(botToken, botName, botUsername);
String botCommand;
String botResponse;
String sender;
String chatId;
unsigned long botScanInterval = 1;
unsigned long botLastScan = 0;
unsigned long nowMillis = 0;
// Add your Telegram user IDs as authorized users
String users[] = {"YOUR_TELEGRAM_ID_1", "YOUR_TELEGRAM_ID_2"};
const int userCount = sizeof(users) / sizeof(users[0]);

void setup()
{
  Serial.begin(115200);
  NetworkManager::init(ssid, password);
  EspNowManager::init();
  ActuatorControl::init();
  TelegramBotManager::init();
}

void loop()
{
  unsigned long currentTime = millis();
  // Throttle error notifications to every 10 minutes
  if (errorInfo.hasError && currentTime - errorInfo.lastErrorTime >= errorInfo.errorTimeInterval)
  { // 600,000 ms is 10 minutes
    TelegramBotManager::sendErrorNotification(errorInfo.sensorId);
    errorInfo.hasError = false; // Reset the error flag after handling

    errorInfo.lastErrorTime = currentTime;
  }

  // Logic for handling data and controlling actuators is encapsulated in their respective modules
  TelegramBotManager::handleMessages();
  FuzzyLogicController::checkAndPerformWatering();
  delay(1000);
}
