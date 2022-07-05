#include <ArduinoWebsockets.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"

// Setup Vars
const char *ssid = "MAI_SWITCHER";
const char *password = "admin77477";
IPAddress ip(192, 168, 4, 1);
IPAddress netmask(255, 255, 255, 0);
int CLEARING_TIME = 1000;
int LINE1_TIME = 7500;
int LINE2_TIME = 5000
int POS_1 = 0;
int POS_2 = 30;
int POS_SAFE = 15;
bool GO_MANUAL = false;

// Pins
#define PLC1_IN 01
#define PLC2_IN 02
#define RELAY1_OUT 03
#define RELAY2_OUT 04
#define SERVO_OUT 05

//libs
Servo switcher; 

// Local Vars
bool LINE_1 = true;
bool LINE_2 = true;
bool AUTO_PROCESS = true;
int PLC1Val = 0;
int PLC2Val = 0;
int REL1Val = LOW;
int REL2Val = LOW;
int ServoVal = 0;

void setup()
{
  	// Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	switcher.setPeriodHertz(50);    // standard 50 hz servo
	switcher.attach(SERVO_OUT, 500, 2400);

  delay(1000);
  Serial.begin(115200);
  Serial.println("Starting Wifi");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip, ip, netmask);
  WiFi.softAP(ssid, password);
}

void loop()
{
  // put your main code here, to run repeatedly:

  process();
}

void process()
{
  // PLC values
  PLC1Val = analogRead(PLC1_IN);
  PLC2Val = analogRead(PLC2_IN);
  if (PLC1Val > 2000 || PLC2Val > 2000)
  {
    AUTO_PROCESS = false;
  }
  else
  {
    if (!AUTO_PROCESS) {
      //Init Default positions
    }
    AUTO_PROCESS = true;
  }
  if (GO_MANUAL){

  } else if (AUTO_PROCESS) {
    startAutoSwitching();
  }
}
void closeLines() {
  digitalWrite(RELAY1_OUT, HIGH);
  digitalWrite(RELAY2_OUT, HIGH);
  delay(CLEARING_TIME);
}
void openLine(int line){
  switch (line)
  {
  case 1:
    switcher.write(POS_1);
    digitalWrite(RELAY1_OUT, HIGH);
    break;
  case 2:
    switcher.write(POS_2);
    digitalWrite(RELAY2_OUT, HIGH);
    break;
  
  default:
    break;
  }
}
void defaultPositions(){
  // Turn Line 1 and 2 OFF and Switch Servo to Line 1.
  closeLines();
  switcher.write(POS_1);
}

void startAutoSwitching(){
  // Seq_1
    // Turn Line 1 and 2 OFF and wait clearing Seconds
    closeLines();
    // Switch Servo to Line 1 and Turn Line 1 ON
    openLine(1);
    // Wait for Line 1 time.
    delay(LINE1_TIME);
  // Seq_2
    closeLines();
    // Turn Line 1 OFF and wait clearing Seconds
    openLine(2);
    // Switch Servo to Line 2 and Turn Line 2 On
    delay(LINE2_TIME);
}

void readPrefs(){
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  File file = SPIFFS.open("/config.json");
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  
  Serial.println("File Content:");
  while(file.available()){
    char JSONContent[] = file.read();
    loadJsonContent(JSONContent);
  }
  file.close();
}
void savePrefs(){

}

loadJsonConent(char data[]){
  StaticJsonBuffer<300> JSONBuffer;//Memory pool
  JsonObject& parsed = JSONBuffer.parseObject(data);
 
  if (!parsed.success()) {   //Check for errors in parsing
    Serial.println("Parsing failed");
    return; 
  }
 
  const char * sensorType = parsed["SensorType"];
  int value = parsed["Value"];
}