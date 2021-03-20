/*
 Main Controller
 Created: Mar 20, 2021 by Isuru Wijegunasinghe
*/

#include "ThingSpeak.h"
#include "secrets.h"

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myReadAPIKey = SECRET_READ_APIKEY;

#include <ESP8266WiFi.h>

char ssid[] = SECRET_SSID;   // network SSID
char pass[] = SECRET_PASS;   // network password
WiFiClient client;

/* ZMPT101B Voltage Sensor */
int sensor = A0;
long val = 0;

void setup() {
  Serial.begin(115200);
  delay(100);

  WiFi.mode(WIFI_STA);

  ThingSpeak.begin(client);

  pinMode(sensor, INPUT);
}

void loop() {

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }

  // Voltage reading
  val = analogRead(sensor);
  Serial.println(val);

  // Write Voltage value to Field 1 of a ThingSpeak Channel
  int httpCode1 = ThingSpeak.writeField(myChannelNumber, 1, val, myWriteAPIKey);
  
  // Read value of Field 2 of a ThingSpeak Channel
  long loadCurrent1 = ThingSpeak.readLongField(myChannelNumber, 2, myReadAPIKey);
  Serial.println("Load 1 Current: " + loadCurrent1);
  int httpCode2 = ThingSpeak.getLastReadStatus();

  if (httpCode2 == 200) {
    Serial.println("Channel read successful.");
  }
  else {
    Serial.println("Problem reading from channel. HTTP error code " + String(httpCode2));
  }

  // Time interval to update the channel
  delay(15000);
}
