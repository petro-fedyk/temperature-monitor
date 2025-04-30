#ifndef CLOCK_H
#define CLOCK_H

#include <Arduino.h>
#include "time.h"
#include "variable.h"

#define UPDATECLOCK 10000
unsigned long currentTime = 0;

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 7200;
const int daylightOffset_sec = 3600;

void setupTime()
{
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
}

void getTime()
{
    if (millis() - currentTime > UPDATECLOCK)
    {
        currentTime = millis();
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {
            Serial.println("No time available (yet)");
            return;
        }

        snprintf(timeBuffer, sizeof(timeBuffer), "%04d.%02d.%02d.%02d:%02d",
                 timeinfo.tm_year + 1900,
                 timeinfo.tm_mday,
                 timeinfo.tm_mon + 1,
                 timeinfo.tm_hour,
                 timeinfo.tm_min);

        Serial.println(timeBuffer);
    }
}

#endif
