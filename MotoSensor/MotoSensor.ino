/***************************************************
  Leistungsarbeit im Fach Embedded Systems
  Projektbeteiligte: Pascal Rusca // Janik Schilter
  ======================
  Messung von Temperatur, Feuchtigkeit und Batteriespannung
  janik.schilter@edu.teko.ch
  pascal.rusca@edu.teko.ch

  220919
  v01_00: Initialversion
  v01_01: Implementierung der Spannungsmessung und Anbindung MQTT
 ***************************************************/

///////////////////////////////////////////////////////////////////
// Bibliotheken
#include <M5Stack.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"
#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <AsyncMqttClient.h>

Adafruit_AM2320 am2320 = Adafruit_AM2320();

///////////////////////////////////////////////////////////////////
// #define
#define WIFI_SSID "L-TEKO"
#define WIFI_PASSWORD "teko2016"

// Raspberry Pi Mosquitto MQTT Broker
#define MQTT_HOST IPAddress(192, 168, 1, 113)
#define MQTT_PORT 1883

// Temperature MQTT Topics
#define MQTT_PUB_TEMP "esp32/dht/temperature"
#define MQTT_PUB_HUM  "esp32/dht/humidity"
#define MQTT_PUB_VOLT "esp32/dht/voltage"

// Analog pin connected to the AM2320-sensor
#define AM2320_PIN 21, 22 
#define DHTTYPE AM2302 

///////////////////////////////////////////////////////////////////
// Variablen und Objekte
  int temp;
  int hum; 
  int batVoltage;
  float voltage;

  AsyncMqttClient mqttClient;
  TimerHandle_t mqttReconnectTimer;
  TimerHandle_t wifiReconnectTimer;

  unsigned long previousMillis = 0;   // Stores last time temperature was published
  const long interval = 10000;        // Interval at which to publish sensor readings

  void connectToWifi() {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

  void connectToMqtt() {
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}

  void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}

  void onMqttConnect(bool sessionPresent) {
    Serial.println("Connected to MQTT.");
    Serial.print("Session present: ");
    Serial.println(sessionPresent);
}

  void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.println("Disconnected from MQTT.");
    if (WiFi.isConnected()) {
      xTimerStart(mqttReconnectTimer, 0);
    }
}

  void onMqttPublish(uint16_t packetId) {
    Serial.print("Publish acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
  }

///////////////////////////////////////////////////////////////////
// setup()  
void setup() {
  Serial.begin(115200);
  Serial.println();

  while (!Serial) {
    delay(10); // hang out until serial port opens
  }

  Serial.println("Adafruit AM2320 Basic Test");
  am2320.begin();


  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials("admin", "6010Kriens");
  connectToWifi();
  }
  
///////////////////////////////////////////////////////////////////
// loop()
void loop() {

/////////////////////////////////
// Eingabe  

  // read temperature from sensor am2320
  temp = am2320.readTemperature(); 
  
  // read humidity from sensor am2320   
  hum = am2320.readHumidity();    
     
  // read the input on analog pin G35
  batVoltage = analogRead(G35);          

  // Check if any reads failed and exit early (to try again).
  if (isnan(temp) || isnan(hum)) {
    Serial.println(F("Failed to read from AM2320 sensors!"));
    return;
  }
  if (isnan(batVoltage)) {
    Serial.println(F("Failed to read from voltage sensor!"));
    return;
  }

/////////////////////////////////
// Verarbeitung
  voltage = batVoltage * (5.0 / 1023.0);  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):

/////////////////////////////////
// Ausgabe
  unsigned long currentMillis = millis();
  // Every X number of seconds (interval = 10 seconds) 
  // it publishes a new MQTT message
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    
    // Publish an MQTT message on topic esp32/dht/temperature
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temp).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_TEMP, packetIdPub1);
    Serial.printf("Message: %.2f \n", temp);

    // Publish an MQTT message on topic esp32/dht/humidity
    uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(hum).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_HUM, packetIdPub2);
    Serial.printf("Message: %.2f \n", hum);

    // Publish an MQTT message on topic esp32/dht/voltage
    uint16_t packetIdPub3 = mqttClient.publish(MQTT_PUB_VOLT, 1, true, String(voltage).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_VOLT, packetIdPub3);
    Serial.printf("Message: %.2f \n", voltage);    
  }


/////////////////////////////////
// Debug
  //Serial.print("temp: "); Serial.println(temp);
  //Serial.print("hum: "); Serial.println(hum);
  //Serial.print("bat_voltage: "); Serial.println(voltage);

  //delay(2000);
}
