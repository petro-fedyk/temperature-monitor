#ifndef SEND_TO_BACKEND
#define SEND_TO_BACKEND

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "convertJson.h"

const char *serverName = "http://192.168.31.159:5000/api/tag_data/";

void sendToServer(const String &timeBuffer, float temperatureC, float maxTemperature, float minTemperature, bool isMaxAlarm, bool isMinAlarm)
{
    HTTPClient http;
    WiFiClient client;

    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");

    String jsonStr = convertToJson(timeBuffer, temperatureC, maxTemperature, minTemperature, isMaxAlarm, isMinAlarm);

    int httpResponseCode = http.POST(jsonStr);

    Serial.print("Sending data: ");
    Serial.println(jsonStr);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0)
    {
        String response = http.getString();
        Serial.print("Response from server: ");
        Serial.println(response);
    }
    else
    {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }

    http.end();
}

#endif
