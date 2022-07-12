
// #ifndef WSHANDLER_H_
// #define WSHANDLER_H_


// extern ESPAsyncDDP  ddp;        // ESPAsyncDDP with X buffers
// extern config_t     config;     // Current configuration
// extern uint32_t     *seqError;  // Sequence error tracking for each universe
// extern uint16_t     uniLast;    // Last Universe to listen for
// extern bool         reboot;     // Reboot flag

// extern const char CONFIG_FILE[];

// /*
//   Packet Commands
//     E1 - Get Elements

//     G1 - Get Config
//     G2 - Get Config Status
//     G3 - Get Current Effect and Effect Config Options

//     V1 - View Stream

//     S1 - Set Network Config
//     S2 - Set Device Config

//     X6 - Reboot
// */

// EFUpdate efupdate;
// uint8_t * WSframetemp;
// uint8_t * confuploadtemp;

// void procX(uint8_t *data, AsyncWebSocketClient *client) {
//     if (data == 'X6'){
//         reboot = true;
//     }
// }

// void procG(uint8_t *data, AsyncWebSocketClient *client) {
//     switch (data[1]) {
//         case '1': {
//             String response;
//             serializeConfig(response, false, true);
//             client->text("G1" + response);
//             break;
//         }

//         case '2': {
//             // Create buffer and root object
//             DynamicJsonDocument json(1024);

//             json["ssid"] = (String)WiFi.SSID();
//             json["hostname"] = (String)WiFi.hostname();
//             json["ip"] = WiFi.localIP().toString();
//             json["mac"] = WiFi.macAddress();
//             json["version"] = (String)VERSION;
//             json["built"] = (String)BUILD_DATE;
//             json["flashchipid"] = String(ESP.getFlashChipId(), HEX);
//             json["usedflashsize"] = (String)ESP.getFlashChipSize();
//             json["realflashsize"] = (String)ESP.getFlashChipRealSize();
//             break;
//         }


//     }
// }

// void procS(uint8_t *data, AsyncWebSocketClient *client) {

//     DynamicJsonDocument json(1024);
//     DeserializationError error = deserializeJson(json, reinterpret_cast<char*>(data + 2));

//     if (error) {
//         LOG_PORT.println(F("*** procS(): Parse Error ***"));
//         LOG_PORT.println(reinterpret_cast<char*>(data));
//         return;
//     }

//     bool reboot = false;
//     switch (data[1]) {
//         case '1':   // Set Network Config
//             dsNetworkConfig(json.as<JsonObject>());
//             saveConfig();
//             client->text("S1");
//             break;
//         case '2':   // Set Device Config
//             // Reboot if MQTT changed
//             if (config.mqtt != json["mqtt"]["enabled"])
//                 reboot = true;

//             dsDeviceConfig(json.as<JsonObject>());
//             saveConfig();

//             if (reboot)
//                 client->text("S1");
//             else
//                 client->text("S2");
//             break;
//         case '3':   // Set Effect Startup Config
//             dsEffectConfig(json.as<JsonObject>());
//             saveConfig();
//             client->text("S3");
//             break;
// #if defined(ESPS_MODE_PIXEL)
//         case '4':   // Set Gamma (but no save)
//             dsGammaConfig(json.as<JsonObject>());
//             client->text("S4");
//             break;
// #endif
//     }
// }

// // void handle_fw_upload(AsyncWebServerRequest *request, String filename,
// //         size_t index, uint8_t *data, size_t len, bool final) {
// //     if (!index) {
// //         WiFiUDP::stopAll();
// //         LOG_PORT.print(F("* Upload Started: "));
// //         LOG_PORT.println(filename.c_str());
// //         efupdate.begin();
// //     }

// //     if (!efupdate.process(data, len)) {
// //         LOG_PORT.print(F("*** UPDATE ERROR: "));
// //         LOG_PORT.println(String(efupdate.getError()));
// //     }

// //     if (efupdate.hasError())
// //         request->send(200, "text/plain", "Update Error: " +
// //                 String(efupdate.getError()));

// //     if (final) {
// //         LOG_PORT.println(F("* Upload Finished."));
// //         efupdate.end();
// //         SPIFFS.begin();
// //         saveConfig();
// //         reboot = true;
// //     }
// // }

// void wsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
//         AwsEventType type, void * arg, uint8_t *data, size_t len) {
//     switch (type) {
//         case WS_EVT_DATA: {
//             AwsFrameInfo *info = static_cast<AwsFrameInfo*>(arg);
//             if (info->opcode == WS_TEXT) {
//                 switch (data[0]) {
//                     case 'X':
//                         procX(data, client);
//                         break;
//                     case 'E':
//                         procE(data, client);
//                         break;
//                     case 'G':
//                         procG(data, client);
//                         break;
//                     case 'S':
//                         procS(data, client);
//                         break;
//                     case 'T':
//                         procT(data, client);
//                         break;
//                     case 'V':
//                         procV(data, client);
//                         break;
//                 }
//             } else {
//                 LOG_PORT.println(F("-- binary message --"));
//             }
//             break;
//         }
//         case WS_EVT_CONNECT:
//             LOG_PORT.print(F("* WS Connect - "));
//             LOG_PORT.println(client->id());
//             break;
//         case WS_EVT_DISCONNECT:
//             LOG_PORT.print(F("* WS Disconnect - "));
//             LOG_PORT.println(client->id());
//             break;
//         case WS_EVT_PONG:
//             LOG_PORT.println(F("* WS PONG *"));
//             break;
//         case WS_EVT_ERROR:
//             LOG_PORT.println(F("** WS ERROR **"));
//             break;
//     }
// }

// #endif /* ESPIXELSTICK_H_ */
