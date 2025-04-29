#ifndef STORAGE_H
#define STORAGE_H
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "convertJson.h"

String readData();

// String jsonStr = convertToJson(timeBuffer, temperatureC, maxTemperature, minTemperature, isMaxAlarm, isMinAlarm);

String readData()
{
    File file = LittleFS.open("/data.json", "r");
    if (!file)
    {
        Serial.println("File not found create new one");
        file = LittleFS.open("/data.json", "w");
        if (file)
        {
            file.println({"file: create, success"}); // write the default pin
            file.close();
            Serial.println("File created");
        }
        else
        {
            Serial.println("Failed to create file");
            return "";
        }
    }
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return "";
    }

    String readJson = "";
    while (file.available())
    {
        readJson += (char)file.read();
    }
    file.close();

    Serial.println("file content: ");
    Serial.println(readJson);
    return readJson;
}

void writeData(const String &json)
{
    File file = LittleFS.open("/data.json", "a");
    if (!file)
    {
        Serial.println("Failed to open file for appending");
        return;
    }
    file.println(json);
    file.close();
    Serial.println("Data appended to file");
}

void storageSetUp()
{
    if (!LittleFS.begin())
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

    Dir dir = LittleFS.openDir("/");

    while (dir.next())
    {
        String fileName = dir.fileName();
        Serial.print("FILE: ");
        Serial.println(fileName);
    }

    readData();
}

#endif