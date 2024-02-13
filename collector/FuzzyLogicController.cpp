#include <Arduino.h>
#include "FuzzyLogicController.h"
#include "TelegramBotManager.h"
#include "ActuatorControl.h"
#include "Globals.h"

// Update these constants based on your fuzzy logic thresholds
const float TEMP_COLD_MAX = 26.0;
const float TEMP_MODERATE_MIN = 26.0;
const float TEMP_MODERATE_MAX = 30.0;
const float TEMP_HOT_MIN = 28.0;

const float HUMIDITY_DRY_MAX = 60.0;
const float HUMIDITY_NORMAL_MIN = 60.0;
const float HUMIDITY_NORMAL_MAX = 80.0;
const float HUMIDITY_WET_MIN = 70.0;

// Define the output levels
const unsigned long MIN_WATERING_TIME = 45000; // 45 seconds
const unsigned long MAX_WATERING_TIME = 90000; // 90 seconds

// Variables for fuzzy logic membership values
static float coldTemp, moderateTemp, hotTemp;
static float dryHumidity, normalHumidity, wetHumidity;
// Variables for fuzzy logic rule outputs
static float ruleOutputs[9], zValues[9][2];

// Function prototypes
static void evaluateTemperatureMembership(float temperature);
static void evaluateHumidityMembership(float humidity);
static float defuzzify();

void FuzzyLogicController::checkAndPerformWatering()
{
    unsigned long currentTime = millis();

    if (globalData.autoWateringEnabled && !globalData.isWatering &&
        currentTime - globalData.lastAutoWateringTime >= 3600000)
    {

        // Here, call processFuzzy() to determine the watering duration
        processFuzzy();
        float wateringDuration = globalData.wateringDuration;

        if (wateringDuration > 0)
        {
            TelegramBotManager::sendWateringNotification("Mulai menyiram selama " + String(wateringDuration / 1000));
            Serial.println("Mulai menyiram selama ");
            Serial.print(wateringDuration / 1000);
            Serial.println(" detik");

            // Start watering
            ActuatorControl::controlPump(true);
            globalData.wateringStartTime = currentTime;
            globalData.isWatering = true;
            globalData.lastAutoWateringTime = currentTime; // Update the last watering time
        }
    }

    // Check if it's time to stop watering
    if (globalData.isWatering && currentTime - globalData.wateringStartTime >= globalData.wateringDuration)
    {
        ActuatorControl::controlPump(false); // Stop watering
        globalData.isWatering = false;
        TelegramBotManager::sendWateringNotification("Penyiraman selesai.");
        return;
    }
}

void FuzzyLogicController::processFuzzy()
{
    // copy the global data to local variables
    float avgTemperature = globalData.averageTemperature;
    float avgHumidity = globalData.averageHumidity;

    // Print the average readings
    Serial.print("Average Temperature = ");
    Serial.print(avgTemperature);
    Serial.print(", Average Humidity = ");
    Serial.println(avgHumidity);

    // Evaluate the fuzzy logic membership values based on averages
    evaluateTemperatureMembership(avgTemperature);
    evaluateHumidityMembership(avgHumidity);

    // Defuzzify to get the actuator control output (e.g., watering duration)
    float wateringDuration = defuzzify();
    globalData.wateringDuration = wateringDuration;

    // Print the defuzzification result
    Serial.print("Watering Duration = ");
    Serial.println(wateringDuration);
}

// Fuzzy logic functions
void evaluateTemperatureMembership(float avgTemperature)
{
    // Cool
    coldTemp = avgTemperature <= TEMP_COLD_MAX ? 1 : (avgTemperature < TEMP_HOT_MIN ? (TEMP_HOT_MIN - avgTemperature) / (TEMP_HOT_MIN - TEMP_COLD_MAX) : 0);

    // Moderate
    moderateTemp = avgTemperature <= TEMP_MODERATE_MIN ? 0 : (avgTemperature < TEMP_HOT_MIN ? (avgTemperature - TEMP_MODERATE_MIN) / (TEMP_HOT_MIN - TEMP_MODERATE_MIN) : (avgTemperature <= TEMP_MODERATE_MAX ? (TEMP_MODERATE_MAX - avgTemperature) / (TEMP_MODERATE_MAX - TEMP_HOT_MIN) : 0));

    // Hot
    hotTemp = avgTemperature < TEMP_HOT_MIN ? 0 : (avgTemperature <= TEMP_MODERATE_MAX ? (avgTemperature - TEMP_HOT_MIN) / (TEMP_MODERATE_MAX - TEMP_HOT_MIN) : 1);
}

void evaluateHumidityMembership(float avgHumidity)
{
    // Dry
    dryHumidity = avgHumidity <= HUMIDITY_DRY_MAX ? 1 : (avgHumidity <= HUMIDITY_WET_MIN ? (HUMIDITY_WET_MIN - avgHumidity) / (HUMIDITY_WET_MIN - HUMIDITY_DRY_MAX) : 0);

    // Normal
    normalHumidity = avgHumidity <= HUMIDITY_NORMAL_MIN ? 0 : (avgHumidity < HUMIDITY_WET_MIN ? (avgHumidity - HUMIDITY_NORMAL_MIN) / (HUMIDITY_WET_MIN - HUMIDITY_NORMAL_MIN) : (avgHumidity <= HUMIDITY_NORMAL_MAX ? (HUMIDITY_NORMAL_MAX - avgHumidity) / (HUMIDITY_NORMAL_MAX - HUMIDITY_WET_MIN) : 0));

    // Wet
    wetHumidity = avgHumidity < HUMIDITY_WET_MIN ? 0 : (avgHumidity <= HUMIDITY_NORMAL_MAX ? (avgHumidity - HUMIDITY_WET_MIN) / (HUMIDITY_NORMAL_MAX - HUMIDITY_WET_MIN) : 1);
}

float minimum(float a, float b)
{
    return (a < b) ? a : b;
}

void evaluateRules()
{
    // Calculate rule strengths
    ruleOutputs[0] = minimum(coldTemp, dryHumidity);        // Cool and Dry -> A Little
    ruleOutputs[1] = minimum(coldTemp, normalHumidity);     // Cool and Normal -> Off
    ruleOutputs[2] = minimum(coldTemp, wetHumidity);        // Cool and Wet -> Off
    ruleOutputs[3] = minimum(moderateTemp, dryHumidity);    // Moderate and Dry -> Off
    ruleOutputs[4] = minimum(moderateTemp, normalHumidity); // Moderate and Normal -> Off
    ruleOutputs[5] = minimum(moderateTemp, wetHumidity);    // Moderate and Wet -> Off
    ruleOutputs[6] = minimum(hotTemp, dryHumidity);         // Hot and Dry -> A Lot
    ruleOutputs[7] = minimum(hotTemp, normalHumidity);      // Hot and Normal -> A Little
    ruleOutputs[8] = minimum(hotTemp, wetHumidity);         // Hot and Wet -> Off
    // rule 1
    zValues[0][0] = (MIN_WATERING_TIME * ruleOutputs[0]) + MIN_WATERING_TIME;
    zValues[0][1] = MAX_WATERING_TIME - (MIN_WATERING_TIME * ruleOutputs[0]);
    // rule 2
    zValues[1][0] = 0;
    // rule 3
    zValues[2][0] = 0;
    // rule 4
    zValues[3][0] = (MIN_WATERING_TIME * ruleOutputs[0]) + MIN_WATERING_TIME;
    zValues[3][1] = MAX_WATERING_TIME - (MIN_WATERING_TIME * ruleOutputs[0]);
    // rule 5
    zValues[4][0] = 0;
    // rule 6
    zValues[5][0] = 0;
    // rule 7
    zValues[6][0] = (MIN_WATERING_TIME * ruleOutputs[6]) + MIN_WATERING_TIME;
    // rule 8
    zValues[7][0] = (MIN_WATERING_TIME * ruleOutputs[0]) + MIN_WATERING_TIME;
    zValues[7][1] = MAX_WATERING_TIME - (MIN_WATERING_TIME * ruleOutputs[0]);
    // rule 9
    zValues[8][0] = 0;
}

float defuzzify()
{
    // Implement defuzzification here
    evaluateRules();

    float A = ((ruleOutputs[0] * zValues[0][0]) + (ruleOutputs[0] * zValues[0][1]) + (ruleOutputs[1] * zValues[1][0]) + (ruleOutputs[2] * zValues[2][0]) + (ruleOutputs[3] * zValues[3][0]) + (ruleOutputs[3] * zValues[3][1]) + (ruleOutputs[4] * zValues[4][0]) + (ruleOutputs[5] * zValues[5][0]) + (ruleOutputs[6] * zValues[6][0]) + (ruleOutputs[7] * zValues[7][0]) + (ruleOutputs[7] * zValues[7][1]) + (ruleOutputs[8] * zValues[8][0]));

    float B = (ruleOutputs[0] + ruleOutputs[1] + ruleOutputs[2] + ruleOutputs[3] + ruleOutputs[4] + ruleOutputs[5] + ruleOutputs[6] + ruleOutputs[7] + ruleOutputs[8]);

    // prevent division by zero
    if (B == 0)
        return 0;

    return A / B;
}
