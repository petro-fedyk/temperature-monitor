#ifndef CONVERTJSON_H
#define CONVERTJSON_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include "clock.h"
#include "readTemapature.h"
#include "variable.h"

String convertToJson(const String &timeBuffer, float temperatureC, float maxTemperature, float minTemperature, bool isMaxAlarm, bool isMinAlarm)
{
    JsonDocument doc;

    doc["time"] = timeBuffer;
    doc["temperatureC"] = temperatureC;
    doc["maxTemperature"] = maxTemperature;
    doc["minTemperature"] = minTemperature;
    doc["isMaxAlarm"] = isMaxAlarm;
    doc["isMinAlarm"] = isMinAlarm;

    String output;
    serializeJson(doc, output);
    return output;
}

// Для відлагодження
void printJson(const String &json)
{
    Serial.println(json);
}

#endif