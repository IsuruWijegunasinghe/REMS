/*
 Main Controller
 Created: Mar 20, 2021 by Isuru Wijegunasinghe
*/

#define zmpt101b A0
#include "ThingSpeak.h"
#include "secrets.h"
#include <ESP8266WiFi.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> 
#define TFT_RST   D4
#define TFT_CS    D3
#define TFT_DC    D2

#define gridAvailability  D0

#define batChargeRelay    D1
#define standInvRelay     D2
#define gridTieInvRelay   D3

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
const char * myReadAPIKey = SECRET_READ_APIKEY;

char ssid[] = SECRET_SSID;   // network SSID
char pass[] = SECRET_PASS;   // network password
WiFiClient client;

/* ZMPT101B Voltage Sensor */
float voltage = 0;

const int noOfLoads = 4;
int priorityOrder[] = {1, 2, 3, 4};
boolean isCharging;
double batteryPowerLevel;

String state = "00000000";

int x = 0;
int y = 0;

void setup() {
  Serial.begin(115200);
  delay(100);

  WiFi.mode(WIFI_STA);

  ThingSpeak.begin(client);

  pinMode(sensor, INPUT);
  pinMode(gridAvailability, INPUT);
  pinMode(batChargeRelay, OUTPUT);
  pinMode(standInvRelay, OUTPUT);
  pinMode(gridTieInvRelay, OUTPUT);
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

  float loadCurrents[noOfLoads];
  
  // Read Load current values from ThingSpeak Channel
  for (int i = 0; i <= noOfLoads; i++) {
    loadCurrents[i] = ThingSpeak.readLongField(myChannelNumber, i+2, myReadAPIKey);
    Serial.println("Load " + String(i+1) + " Current: " + String(loadCurrents[i]));
    int httpCode2 = ThingSpeak.getLastReadStatus();
  
    if (httpCode2 == 200) {
      Serial.println("Load " + String(i+1) + " Current read successful.");
    }
    else {
      Serial.println("Problem reading Load " + String(i+1) + " Current. HTTP error code " + String(httpCode2));
    }
  }
  
  float loadPowerConsumptions[noOfLoads];
  for (int i = 0; i <= noOfLoads; i++) {
    loadPowerConsumptions[i] = 230.0 * loadCurrents[i];
  }

  // Set battery to charging state
  isCharging = true;

  float loadsInPriority[noOfLoads];
  for (int i = 0; i <= noOfLoads; i++) {
    loadsInPriority[i] = loadPowerConsumptions[priorityOrder[i]];
  }
  
  if (gridAvailability) {
    if (isCharging){
      digitalWrite(batChargeRelay, HIGH);
      digitalWrite(gridTieInvRelay, HIGH);
      digitalWrite(standInvRelay, LOW);
      // Connect all loads to grid
      state = "01010101"
      ThingSpeak.writeField(myChannelNumber, 6, state, myWriteAPIKey);
    } else {
      x = 0;
      loadSum = loadsInPriority[noOfLoads - x - 1];
      while(batteryPowerLevel > loadSum){
        x = x + 1;
        loadSum = loadSum + loadsInPriority[noOfLoads - x - 1];
      }
      digitalWrite(batChargeRelay, LOW);
      digitalWrite(standInvRelay, HIGH);
      digitalWrite(gridTieInvRelay, HIGH);
      // Connect x loads to Battery (Least Priority)
      /* Add code */
    }
  } else {
    y = 0;
    loadSum = loadsInPriority[0];
    while(batteryPowerLevel > loadSum){
      y = y + 1;
      loadSum = loadSum + loadsInPriority[y];
    }
    digitalWrite(batChargeRelay, LOW);
    digitalWrite(standInvRelay, HIGH);
    digitalWrite(gridTieInvRelay, LOW);
    // Connect y loads to Battery (Highest Priority)
    /* Add code */
  }

  // Time interval to update the channel
  delay(15000);
}
