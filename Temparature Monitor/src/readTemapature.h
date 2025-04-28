#ifndef READTEMAPATURE_H
#define READTEMAPATURE_H
#include <Arduino.h>
#include "variable.h"
#include <OneWire.h>
#include <DallasTemperature.h>
extern DallasTemperature sensors;

float readDSTemperatureC()
{
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    if (tempC == -127.00)
    {
        Serial.println("Failed to read from DS18B20 sensor");
        return NAN;
    }
    else
    {
        Serial.print("Temperature Celsius: ");
        Serial.println(tempC);
    }
    return tempC;
}

// min function
void updateMinTemp(float tempC)
{
    // minTemperature = readDSTemperatureC();
    if (tempC < minTemperature)
    {
        minTemperature = tempC;
    }
    Serial.print("Min Temperature: ");
    Serial.println(minTemperature);
}

// max function
void updateMaxTemp(float tempC)
{
    // maxTemperature = readDSTemperatureC();
    if (tempC > maxTemperature)
    {
        maxTemperature = tempC;
    }
    Serial.print("Max Temperature: ");
    Serial.println(maxTemperature);
}
// alarm fun
void checkAlarm(float tempC)
{
    if (tempC > maxTempAlarm)
    {
        isMaxAlarm = true;
        Serial.println("Temperature max alarm triggered!");
    }
    else if (tempC < minTempAlarm)
    {
        isMaxAlarm = true;
        Serial.println("Temperature alarm min triggered!");
    }
    return;
}

// Replaces placeholder with DS18B20 values on the web page
String processor(const String &var)
{
    if (var == "TEMPERATUREC")
    {
        return String(temperatureC, 2); // 2 знаки після крапки
    }
    else if (var == "MINTEMP")
    {
        return String(minTemperature, 2);
    }
    else if (var == "MAXTEMP")
    {
        return String(maxTemperature, 2);
    }
    else if (var == "ALARM")
    {
        if (isMaxAlarm)
        {
            return String("Max Alarm Triggered");
        }
        else if (isMinAlarm)
        {
            return String("Min Alarm Triggered");
        }
        else
        {
            return String("No Alarm");
        }
    }
    return String();
}

#endif