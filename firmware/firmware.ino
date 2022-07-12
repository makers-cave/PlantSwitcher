#include <Nextion.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>




// Setup Vars
#define HTTP_PORT       80 
const char *ssid = "MAI_SWITCHER";
const char *password = "admin77477";
IPAddress ip(192, 168, 4, 1);
IPAddress netmask(255, 255, 255, 0);
#define LOG_PORT        Serial
int CLEARING_TIME = 1000;
int LINE1_TIME = 7500;
int LINE2_TIME = 5000;
int POS_1 = 0;
int POS_2 = 30;
int POS_SAFE = 15;
bool GO_MANUAL = false;
AsyncWebServer      web(HTTP_PORT); // Web Server
AsyncWebSocket      ws("/ws");      // Web Socket Plugin


//NexPage page0    = NexPage(0, 0, "Home");
//NexButton btnAuto = NexButton(0, 2, "btnAuto");
//NexButton btnManual = NexButton(0, 3, "btnManual");
//NexText stline1 = NexText(0, 4, "stline1");
//NexText stline2 = NexText(0, 5, "stline2");
//NexPicture btna10 = NexPicture(1, 11, "btna10");
//NexPicture btna11 = NexPicture(1, 12, "btna11");
//NexPicture btna20 = NexPicture(1, 18, "btna20");
//NexPicture btna21 = NexPicture(1, 19, "btna21");
//NexButton btnSave = NexButton(1, 14, "btnSave");
//NexNumber xct = NexNumber(1, 13, "xct");
//NexNumber na1 = NexNumber(1, 6, "na1");
//NexNumber na2 = NexNumber(1, 7, "na2");
//NexNumber xrt1 = NexNumber(1, 16, "xrt1");
//NexNumber xrt2 = NexNumber(1, 17, "xrt2");
//
//// Register a button object to the touch event list.  
//NexTouch *nex_listen_list[] = {
//  &btnAuto,
//  &btnManual,
//  &btna10,
//  &btna11,
//  &btna20,
//  &btna21,
//  &btnSave,
//  NULL
//};


// Pins
#define PLC1_IN 26
#define PLC2_IN 25
#define RELAY1_OUT 12
#define RELAY2_OUT 13
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
int CURRENT_LINE =1;

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
  LOG_PORT.begin(115200);
  LOG_PORT.println("Starting Wifi");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip, ip, netmask);
  WiFi.softAP(ssid, password);
  initWeb();
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
    }
    AUTO_PROCESS = true;
  }
  if (GO_MANUAL){
    openLine(CURRENT_LINE);
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
void initWeb() {
  ws.onEvent(wsEvent);
  web.addHandler(&ws);

  // JSON Config Handler
  web.on("/conf", HTTP_GET, [](AsyncWebServerRequest *request) {
      String jsonString;
//      serializeConfig(jsonString, true);
      request->send(200, "text/json", jsonString);
  });

  // Static Handler
  web.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");
}
void wsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
  AwsEventType type, void * arg, uint8_t *data, size_t len){
    switch (type) {
        case WS_EVT_DATA: {
            AwsFrameInfo *info = static_cast<AwsFrameInfo*>(arg);
            if (info->opcode == WS_TEXT) {
                switch (data[0]) {
                    case 'X':
                        procX(data, client);
                        break;
                    case 'G':
                        getData(data, client);
                        break;
                    case 'S':
                        setData(data, client);
                        break;
                }
            } else {
                LOG_PORT.println(F("-- binary message --"));
            }
            break;
        }
        case WS_EVT_CONNECT:
            LOG_PORT.print(F("* WS Connect - "));
            LOG_PORT.println(client->id());
            break;
        case WS_EVT_DISCONNECT:
            LOG_PORT.print(F("* WS Disconnect - "));
            LOG_PORT.println(client->id());
            break;
        case WS_EVT_PONG:
            LOG_PORT.println(F("* WS PONG *"));
            break;
        case WS_EVT_ERROR:
            LOG_PORT.println(F("** WS ERROR **"));
            break;
    }
}
void getData(uint8_t *data, AsyncWebSocketClient *client){
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
  client->text(response);
}
void setData(uint8_t *data, AsyncWebSocketClient *client){
  DynamicJsonDocument json(1024);
  DeserializationError error = deserializeJson(json, reinterpret_cast<char*>(data));
  if (error) {
    LOG_PORT.println(F("*** setData(): Parse Error ***"));
    LOG_PORT.println(reinterpret_cast<char*>(data));
  }
  JsonObject homeData = json["home"];
  if (homeData["MODE"].as<int>() == 1){
    onAutoClick();
  } else {
    onManualClick();
    CURRENT_LINE = homeData["LINE"].as<int>();
  }
  JsonObject configData = json["config"];
  CLEARING_TIME = json["CLEARTIME"].as<float>() * 1000;
  POS_1 = json["L1ANGLE"].as<int>();
  POS_2 = json["L2ANGLE"].as<int>();
  POS_SAFE = (POS_1 + POS_2) / 2;
  LINE1_TIME = json["L1RUNTIME"].as<float>() * 1000;
  LINE2_TIME = json["L2RUNTIME"].as<float>() * 1000;
}
void procX(uint8_t *data, AsyncWebSocketClient *client){
  
}
//void initDisplay(){
//  Serial.begin(115200);
//  nexInit();
//  btnAuto.attachPop(onAutoClick, &btnAuto);
//  btnManual.attachPop(onManualClick, &btnManual);
//  btna10.attachPop(onAngle1MinusClick, &btna10);
//  btna11.attachPop(onAngle1PlusClick, &btna11);
//  btna20.attachPop(onAngle2MinusClick, &btna20);
//  btna21.attachPop(onAngle2PlusClick, &btna21);
//  btnSave.attachPop(onSaveClick, &btnSave);
//}
//void handleDisplay(){
//  
//}
void onAutoClick(){
//  btnManual.Set_background_color_bco(50712);
//  btnAuto.Set_background_color_bco(2016);
  AUTO_PROCESS =true;
  GO_MANUAL = false; 
}
void onManualClick(){
//  btnAuto.Set_background_color_bco(2016);
//  btnManual.Set_background_color_bco(50712);
  AUTO_PROCESS =false;
  GO_MANUAL = true;  
}
//void onAngle1MinusClick(void *ptr){
//  
//}
//void onAngle1PlusClick(void *ptr){
//  
//}
//void onAngle2MinusClick(void *ptr){
//  
//}
//void onAngle2PlusClick(void *ptr){
//  
//}
//void onSaveClick(void *ptr){
//  
//}
