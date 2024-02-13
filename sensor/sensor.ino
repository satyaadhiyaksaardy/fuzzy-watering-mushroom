#include <ESP8266WiFi.h>
#include <espnow.h>
#include "DHT.h"

// Set node id
#define NODE_ID 2

// Insert your SSID
constexpr char WIFI_SSID[] = "YOUR_SSID";

// DHT Sensor setup
#define DHTPIN 5      // Pin where the DHT sensor is connected
#define DHTTYPE DHT11 // Type of DHT sensor

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor

// Struct for sending data
typedef struct
{
  int id;            // Device ID
  float temperature; // Temperature reading
  float humidity;    // Humidity reading
} Message;

Message messageData; // Create an instance of the message struct

// ESP-NOW collector's MAC address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Callback function when data is sent
void onDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0)
  {
    Serial.println("Success");
  }
  else
  {
    Serial.println("Fail");
  }
}

int32_t getWiFiChannel(const char *ssid)
{
  if (int32_t n = WiFi.scanNetworks())
  {
    for (uint8_t i = 0; i < n; i++)
    {
      if (!strcmp(ssid, WiFi.SSID(i).c_str()))
      {
        return WiFi.channel(i);
      }
    }
  }
  return 0;
}

void setup()
{
  Serial.begin(115200); // Start serial communication at 115200 baud rate

  WiFi.mode(WIFI_STA); // Set WiFi mode to Station
  int32_t channel = getWiFiChannel(WIFI_SSID);

  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  wifi_promiscuous_enable(1);
  wifi_set_channel(channel);
  wifi_promiscuous_enable(0);
  WiFi.printDiag(Serial); // Uncomment to verify channel change after

  // Initialize ESP-NOW
  if (esp_now_init() != 0)
  {
    Serial.println("ESP-NOW initialization failed!");
    return;
  }

  // Set ESP-NOW role
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  // Register callback for when data is sent
  esp_now_register_send_cb(onDataSent);

  // Add ESP-NOW peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  // Initialize DHT sensor
  dht.begin();
}

void loop()
{
  static unsigned long lastTime = 0;     // Stores last time a message was sent
  const unsigned long timerDelay = 5000; // Delay between messages

  // Check if it's time to send a new message
  if (millis() - lastTime > timerDelay)
  {
    // Read sensor data
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Prepare data
    messageData.id = NODE_ID; // Device ID, adjust as necessary
    messageData.temperature = isnan(temperature) ? -999 : temperature;
    messageData.humidity = isnan(humidity) ? -999 : humidity;
    //    messageData.temperature = random(2100, 3100) / 100.0; // Simulate temperature
    //    messageData.humidity = random(6000, 9000) / 100.0;    // Simulate humidity

    Serial.println(messageData.temperature);
    Serial.println(messageData.humidity);

    // Send data via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *)&messageData, sizeof(messageData));

    lastTime = millis(); // Update last time message was sent
  }
}
