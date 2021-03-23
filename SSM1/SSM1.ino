/*
 Smart Socket Module 1
 Created: Mar 20, 2021 by Isuru Wijegunasinghe
*/

#define aca712 A0
#include "ThingSpeak.h"
#include "secrets.h"
#include <ESP8266WiFi.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> 
#define TFT_RST   D4
#define TFT_CS    D3
#define TFT_DC    D2

#define batteryRelay  D0
#define gridRelay     D1

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
const char * myReadAPIKey = SECRET_READ_APIKEY;

char ssid[] = SECRET_SSID;   // network SSID
char pass[] = SECRET_PASS;   // network password
WiFiClient client;

/* ACS712 Current Sensor */
int mVperAmp = 185; // use 100 for 20A Module and 66 for 30A Module
float VRMS = 0;
float AmpsRMS = 0;
float AmpsRMS_corrected = 0;
float Voltage = 0;

String statePrev = "00";
String stateNew = "00";

void setup() {
  Serial.begin(115200);
  delay(100);

  WiFi.mode(WIFI_STA);

  ThingSpeak.begin(client);

  pinMode(aca712, INPUT);
  pinMode(batteryRelay, OUTPUT);
  pinMode(gridRelay, OUTPUT);

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_WHITE);

  startup();
  refreshing();
}

void loop() {
  refreshing();

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
  Voltage = getVPP();
  VRMS = (Voltage / 2.0) * 0.707;
  AmpsRMS = (VRMS * 1000) / mVperAmp;
  //AmpsRMS_corrected = (AmpsRMS / 0.94873) - 0.5082;
  Serial.print(AmpsRMS);
  Serial.println(" Amps RMS");

  tftPrintTestAmp();
  tft.setCursor(10, 100);
  tft.print(String(AmpsRMS) + "  A");

  // Write Current value to Field 2 of a ThingSpeak Channel
  int httpCode1 = ThingSpeak.writeField(myChannelNumber, 2, AmpsRMS, myWriteAPIKey);

  if (httpCode1 == 200) {
    Serial.println("Channel write successful.");
  }
  else {
    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode1));
  }

  // Read Relay States from ThingSpeak Channel
  stateNew = String(ThingSpeak.readLongField(myChannelNumber, 6, myReadAPIKey)).substring(0,2); // State as battery, grid
  Serial.println("Relay States: " + stateNew);
  int httpCode2 = ThingSpeak.getLastReadStatus();

  //int batteryRelayState = toInt(state.substring(0,1));
  //int gridRelayState = toInt(state.substring(1,2));

  // Relay controlling
  if (statePrev.equals("00")) {
    if (stateNew.equals("01")) {
      digitalWrite(gridRelay, HIGH);
    } else if (stateNew.equals("10")) {
      digitalWrite(batteryRelay, HIGH);
    }
  } else if (statePrev.equals("01")) {
    if (stateNew.equals("00")) {
      digitalWrite(gridRelay, LOW);
    } else if (stateNew.equals("10")) {
      digitalWrite(batteryRelay, HIGH);
      digitalWrite(gridRelay, LOW);
    }
  } else if (statePrev.equals("10")) {
    if (stateNew.equals("00")) {
      digitalWrite(batteryRelay, LOW);
    } else if (stateNew.equals("01")) {
      digitalWrite(gridRelay, HIGH);
      digitalWrite(batteryRelay, LOW);
    }
  }

  statePrev = stateNew;

  // Time interval to update the channel
  delay(15000);
}

/* For Current measurements */
float getVPP(){
  float result;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;       // store min value here

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000) //sample for 1 Sec
  {
    readValue = analogRead(aca712);
    // see if you have a new maxValue
    if (readValue > maxValue)
    {
      /*record the maximum sensor value*/
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      /*record the minimum sensor value*/
      minValue = readValue;
    }
  }

  // Subtract min from max
  result = ((maxValue - minValue) * 5.0) / 1024.0;

  return result;
}

void tftPrintTestAmp() {
  tft.setCursor(5, 80);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(2);
  tft.print("Current   ");
}

void startup() {
  tft.setCursor(5, 50);
  tft.setTextColor(ST7735_BLACK);
  tft.setTextSize(1.5);
  tft.println("Initializing");
  tft.setCursor(5, 70);
  tft.println("created by G32");
  delay(2000);
}

void refreshing() {
  tft.fillScreen(ST7735_WHITE);
  tft.setCursor(20, 2);
  tft.setTextColor(ST7735_BLACK);
  tft.setTextSize(1);
  tft.println("Load No 1 ");
  tft.setCursor(20, 10);
  tft.println("receiving data");
  tft.setCursor(30, 140);
  tft.setTextSize(1);
  tft.setTextColor(ST7735_BLACK);
  tft.println("FYP G32");
}
