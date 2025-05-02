

#ifdef ESP32
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#else
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>

#endif
#include <OneWire.h>
#include <DallasTemperature.h>

#include <ESP8266mDNS.h>

#include "wifi.h"
#include "index_html.h"
#include "readTemapature.h"
#include "variable.h"
#include "clock.h"
#include "convertJson.h"
#include "storage.h"
#include "sendToBackEnd.h"

#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

unsigned long lastTime = 0;
unsigned long lastJsonTime = 0;

unsigned long timerDelay = 1000;
unsigned long jsonTime = 900000;
// unsigned long jsonTime = 5000;

void updateJson()
{
  if ((millis() - lastJsonTime) > jsonTime)
  {
    String jsonStr = convertToJson(timeBuffer, temperatureC, maxTemperature, minTemperature, isMaxAlarm, isMinAlarm);
    writeData(jsonStr);
    printJson(jsonStr);
    sendToServer(timeBuffer, temperatureC, maxTemperature, minTemperature, isMaxAlarm, isMinAlarm);

    String jsonString = readData();
    Serial.println(jsonString);
    lastJsonTime = millis();
  }
}

void updateTemperature()
{
  if ((millis() - lastTime) > timerDelay)
  {
    float newTemp = readDSTemperatureC();
    if (!isnan(newTemp))
    {
      temperatureC = newTemp;
      updateMinTemp(temperatureC);
      updateMaxTemp(temperatureC);
      checkAlarm(temperatureC);
    }
    lastTime = millis();
  }
}

void setup()
{
  Serial.begin(115200);
  connectWiFi();
  InitMDNS();
  Serial.println();
  storageSetUp();

  sensors.begin();
  temperatureC = readDSTemperatureC();
  setupTime();
}

void loop()
{
  updateJson();
  updateTemperature();
  MDNS.update();
  getTime();
}
