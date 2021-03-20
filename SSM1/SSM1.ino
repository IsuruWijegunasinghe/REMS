/*
 Smart Socket Module 1
 Created: Mar 20, 2021 by Isuru Wijegunasinghe
*/

#include "ThingSpeak.h"
#include "secrets.h"

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

#include <ESP8266WiFi.h>

char ssid[] = SECRET_SSID;   // network SSID
char pass[] = SECRET_PASS;   // network password
WiFiClient client;

/* ACS712 Current Sensor */
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

  // Current reading
  val = analogRead(sensor);
  Serial.println(val);

  // Write Current value to Field 2 of a ThingSpeak Channel
  int httpCode = ThingSpeak.writeField(myChannelNumber, 2, val, myWriteAPIKey);

  if (httpCode == 200) {
    Serial.println("Channel write successful.");
  }
  else {
    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
  }

  // Time interval to update the channel
  delay(15000);
}
