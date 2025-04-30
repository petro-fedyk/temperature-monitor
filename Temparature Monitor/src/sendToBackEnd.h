// #ifndef SEND_TO_BACKEND
// #define SEND_TO_BACKEND

// #include <Arduino.h>
// #include <ESP8266HTTPClient.h>
// #include <WiFiClient.h>
// #include "convertJson.h"

// const char *serverName = "http://192.168.5.1";

// void sendToServer(const String &timeBuffer, float temperatureC, float maxTemperature, float minTemperature, bool isMaxAlarm, bool isMinAlarm)
// {
//     HTTPClient http;
//     WiFiClient client;

//     http.begin(client, serverName);
//     http.addHeader("Content-Type", "application/json");

//     String jsonStr = convertToJson(timeBuffer, temperatureC, maxTemperature, minTemperature, isMaxAlarm, isMinAlarm);

//     http.POST(jsonStr);
//     http.end();

//     Serial.println("Data sent to server.");
// }

// #endif
