#include "createJson.h"

uint16_t logId = 0; // порядковий номер логу

void logToJson()
{
    logId++;

    // Формування JSON-рядка
    String json = "{";
    json += "\"logId\":" + String(logId) + ",";
    json += "\"date\":\"" + String(day) + "." + String(month) + "." + String(year) + "\",";
    json += "\"avgTemp\":" + String(avrTemp, 2) + ",";
    json += "\"minTemp\":" + String(minTemp, 2) + ",";
    json += "\"maxTemp\":" + String(maxTemp, 2) + ",";
    json += "\"timeAboveMax\":" + String(totalHighOutTime / 60000UL) + ",";
    json += "\"timeBelowMin\":" + String(totalLowOutTime / 60000UL);
    json += "}";

    Serial.println("JSON log: ");
    Serial.println(json);
}
