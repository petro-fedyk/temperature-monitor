#ifndef WIFI_H
#define WIFI_H
#define HOST_NAME "fridge-tag"
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "index_html.h"
#include "readTemapature.h"
#include "variable.h"
#include "storage.h"

// Replace with your network credentials
const char *ssid = "admin";
const char *password = "domestos1216";

AsyncWebServer server(80);

void InitMDNS()
{
  if (!MDNS.begin(HOST_NAME))
  {
    Serial.println("Error starting mDNS");
  }
  MDNS.addService("http", "tcp", 80);
  Serial.println("mDNS started");
}

void connectWiFi()
{
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html, processor); });
  server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", String(temperatureC, 2)); });

  server.on("/mintemperature", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", String(minTemperature, 2)); });

  server.on("/maxtemperature", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", String(maxTemperature, 2)); });

  server.on("/alarm", HTTP_GET, [](AsyncWebServerRequest *request)
            {
                String alarmStatus;
                if (isMaxAlarm)
                {
                  alarmStatus = "Max Alarm Triggered";
                }
                else if (isMinAlarm)
                {
                  alarmStatus = "Min Alarm Triggered";
                }
                else
                {
                  alarmStatus = "No Alarm";
                }
                request->send(200, "text/plain", alarmStatus); });

  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/data.json", "application/json", true); });

  server.begin();
}

#endif