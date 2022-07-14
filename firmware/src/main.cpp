//
// A simple server implementation showing how to:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s
//

#include <Arduino.h>
#include <ArduinoJson.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESP32Servo.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <Servo.h>
#endif
#include <ESPAsyncWebServer.h>
#include "LITTLEFS.h"

#define FORMAT_LITTLEFS_IF_FAILED false
AsyncWebServer server(80);
AsyncWebSocket      ws("/ws");      // Web Socket Plugin
Servo switcher; 

#define PLC1_IN 14
#define PLC2_IN 25
#define RELAY1_OUT 12
#define RELAY2_OUT 13
#define SERVO_OUT 05

const char* ssid = "NETGEAR75";
const char* password = "sillytulip502";
int CLEARING_TIME = 1000;
int LINE1_TIME = 7500;
int LINE2_TIME = 5000;
int POS_1 = 0;
int POS_2 = 30;
int POS_SAFE = 15;
bool GO_MANUAL = false;

bool LINE_1 = true;
bool LINE_2 = true;
bool AUTO_PROCESS = true;
int PLC1Val = 0;
int PLC2Val = 0;
int REL1Val = LOW;
int REL2Val = LOW;
int ServoVal = 0;
int CURRENT_LINE =1;

void closeLines() {
  digitalWrite(RELAY1_OUT, HIGH);
  digitalWrite(RELAY2_OUT, HIGH);
  delay(CLEARING_TIME);
}
void openLine(int line){
  CURRENT_LINE = line;
  switch (line)
  {
  case 1:
    switcher.write(POS_1);
    digitalWrite(RELAY1_OUT, HIGH);
    digitalWrite(RELAY2_OUT, LOW);
    break;
  case 2:
    switcher.write(POS_2);
    digitalWrite(RELAY1_OUT, LOW);
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
void process()
{
  // PLC values
  // PLC1Val = analogRead(PLC1_IN);
  // PLC2Val = analogRead(PLC2_IN);
//  Serial.print("PLC 1 Input: ");
//  Serial.println(PLC1Val);
//  Serial.print("PLC 2 Input: ");
//  Serial.println(PLC2Val);
  if (PLC1Val > 2000 || PLC2Val > 2000)
  {
    AUTO_PROCESS = false;
  }
  else
  {
    if (!AUTO_PROCESS) {
      //Init Default positions
      closeLines();
    }
    AUTO_PROCESS = true;
  }
  if (GO_MANUAL){
    openLine(CURRENT_LINE);
  } else if (AUTO_PROCESS) {
    startAutoSwitching();
  }
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}
void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
String createResponse(){
  DynamicJsonDocument json(1024);

  json["MODE"] = GO_MANUAL? 0 : 1; //Automatic
  json["LINE"] = CURRENT_LINE;
  json["CLEARTIME"] = CLEARING_TIME;
  json["L1ANGLE"] = POS_1;
  json["L2ANGLE"] = POS_2;
  json["L1RUNTIME"] = LINE1_TIME;
  json["L2RUNTIME"] = LINE2_TIME;

  String response;
  serializeJson(json, response);
  return response;
}
void readConfig(){
    Serial.printf("Reading Config");
    String output;

    File file = LITTLEFS.open("/config.cfg","r",false);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    DynamicJsonDocument json(1024);
    DeserializationError error = deserializeJson(json, file);
    file.close();
    if (!error){
      CLEARING_TIME = json["CLEARTIME"].as<int>();
      POS_1 = json["L1ANGLE"].as<int>();
      POS_2 = json["L2ANGLE"].as<int>();
      POS_SAFE = (POS_1 + POS_2) / 2;
      LINE1_TIME = json["L1RUNTIME"].as<int>() ;
      LINE2_TIME = json["L2RUNTIME"].as<int>();
    }
    String response;
    serializeJson(json, response);
    Serial.println(response);
}
void saveConfig(){

    File file = LITTLEFS.open("/config.cfg", "w", true);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    String response = createResponse();
    Serial.print("Saving This: ");
    Serial.println(response);
    if(file.print(response)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}
void onAutoClick(){
  Serial.println("Auto Clicked.");
  AUTO_PROCESS =true;
  GO_MANUAL = false; 
}
void onManualClick(){
  AUTO_PROCESS =false;
  GO_MANUAL = true;  
}
void getData(uint8_t *data, AsyncWebSocketClient *client){
  String response = createResponse();
  client->text(response);
}

void procX(uint8_t *data, AsyncWebSocketClient *client){
  
}
void wsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
  AwsEventType type, void * arg, uint8_t *data, size_t len){
    switch (type) {
        case WS_EVT_DATA: {
          Serial.printf("Received data:");
          Serial.println(reinterpret_cast<char*>(data));
          DynamicJsonDocument json(1024);
          DeserializationError error = deserializeJson(json, reinterpret_cast<char*>(data));
          if (error){ return;}

            AwsFrameInfo *info = static_cast<AwsFrameInfo*>(arg);
            if (info->opcode == WS_TEXT) {

              if (json["CMD"].as<String>() == "G"){

                getData(data, client);
              } else {

                // setData(data, client);
                if (json["CMD"].as<String>() == "M")
                {
                  if (json["MODE"].as<int>() == 1){
                    Serial.printf("Mode 1");
                    AUTO_PROCESS = true;
                    closeLines();
                    onAutoClick();
                  } else {
                    Serial.printf("Mode 0");
                    if (!GO_MANUAL){
                      onManualClick();
                    }
                  }
                  
                } else   if (json["CMD"].as<String>() == "L")
                {
                  if (GO_MANUAL){
                    closeLines();
                    openLine(json["LINE"].as<int>());
                  }
                } else if (json["CMD"].as<String>() == "T" || json["CMD"].as<String>() == "S" )
                {
                  Serial.println("SaveProcess");

                  Serial.print("Value of Clear Time:");
                  Serial.println(json["CFG"]["CLEARTIME"].as<int>());

                  CLEARING_TIME = json["CFG"]["CLEARTIME"].as<int>();
                  POS_1 = json["CFG"]["L1ANGLE"].as<int>();
                  POS_2 = json["CFG"]["L2ANGLE"].as<int>();
                  POS_SAFE = (POS_1 + POS_2) / 2;
                  LINE1_TIME = json["CFG"]["L1RUNTIME"].as<int>() ;
                  LINE2_TIME = json["CFG"]["L2RUNTIME"].as<int>();
                  if (json["CMD"].as<String>() == "S") {
                    Serial.println("calling SaveConfig");
                    saveConfig();
                  }
                }
                String response = createResponse();
                client->text(response);
              }
                // switch (json["CMD"].as<char>()) {
                //     case 'X':
                //         procX(data, client);
                //         break;
                //     case 'G':
                //         getData(data, client);
                //         break;
                //     case 'S':
                //     case 'L':
                //         setData(data, client);
                //         break;
                // }
            } else {
                Serial.println(F("-- binary message --"));
            }
            break;
        }
        case WS_EVT_CONNECT:
            Serial.print(F("* WS Connect - "));
            Serial.println(client->id());
            break;
        case WS_EVT_DISCONNECT:
            Serial.print(F("* WS Disconnect - "));
            Serial.println(client->id());
            break;
        case WS_EVT_PONG:
            Serial.println(F("* WS PONG *"));
            break;
        case WS_EVT_ERROR:
            Serial.println(F("** WS ERROR **"));
            break;
    }
  }
void initWeb(){
      if (LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
      Serial.println("Little FS Successfully Mounted.");
      listDir(LITTLEFS, "/" ,2);
    }
    ws.onEvent(&wsEvent);
    server.addHandler(&ws);
    server.serveStatic("/", LITTLEFS, "/").setDefaultFile("index.html");

    server.onNotFound(notFound);

    server.begin();
}
void setup() {
  pinMode(RELAY1_OUT, OUTPUT);
  pinMode(RELAY2_OUT, OUTPUT);
  pinMode(PLC1_IN, INPUT);
  pinMode(PLC2_IN, INPUT);

	switcher.setPeriodHertz(50);    // standard 50 hz servo
	switcher.attach(SERVO_OUT, 500, 2400);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      return;
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  initWeb();
  readConfig();
}

void loop() {
  process();
}