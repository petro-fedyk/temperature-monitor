#include "temperature.h"
#include <Wire.h>

#define SENSOR_ADDR 0x48
#define UPDATE_TIME 2000
// #define LOW_ALARM_TIME (8UL * 60000UL)   // 8 хв
// #define HIGH_ALARM_TIME (15UL * 60000UL) // 15 хв

#define LOW_ALARM_TIME 3000
#define HIGH_ALARM_TIME 4000
// #define HOUR_TIME 3600000 // 1 година
// #define DAY_TIME 86400000 // 1 доба
#define HOUR_TIME 10000 // for debuging
#define DAY_TIME 20000  // for debuging

// глобальні змінні тут
float tempC = 0;
float maxTemp = -1000.0;
float minTemp = 1000.0;
float avrTemp = 0;
float maxTempAlarm = 30;
float minTempAlarm = 20;

// для годинного усереднення
float hourSum = 0;
uint16_t hourCount = 0;
float daySum = 0;
uint8_t hoursPassed = 0;

bool isLowAlarm = false;
bool isHighAlarm = false;

unsigned long currTime = 0;

unsigned long lowOutStart = 0;
unsigned long highOutStart = 0;

unsigned long lastHourTime = 0;
unsigned long lastDayTime = 0;

unsigned long totalLowOutTime = 0;
unsigned long totalHighOutTime = 0;

unsigned long startAlarmTimer = 0;

void readTemp()
{
    uint16_t raw = readRegister16(SENSOR_ADDR, 0x00); // наприклад, регістр температури
    tempC = raw * 0.00390625;                         // формула залежить від датчика

    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println(" C");
}

uint16_t readRegister16(uint8_t address, uint8_t reg)
{
    Wire.beginTransmission(address);
    Wire.write(reg);             // вибираємо регістр
    Wire.endTransmission(false); // повторний старт (не stop)

    Wire.requestFrom(address, (uint8_t)2); // читаємо 2 байти

    uint16_t val = 0;
    if (Wire.available() == 2)
    {
        val = Wire.read() << 8; // старший байт
        val |= Wire.read();     // молодший байт
    }

    return val;
}

void checkAlarm()
{
    unsigned long now = millis();

    // --- Перевірка нижньої межі ---
    if (tempC < minTempAlarm)
    {
        if (lowOutStart == 0) // перший вихід
        {
            lowOutStart = now;
            Serial.println("Temperature below min started");
        }
        else
        {
            // додаємо час у виході
            totalLowOutTime += (now - lowOutStart);
            lowOutStart = now;

            if (!isLowAlarm && (totalLowOutTime >= LOW_ALARM_TIME))
            {
                isLowAlarm = true;
                Serial.println("LOW ALARM: temp below min for >8 minutes");
            }
        }
    }
    else
    {
        // якщо вийшла з зони — обнуляємо старт
        lowOutStart = 0;
    }

    // --- Перевірка верхньої межі ---
    if (tempC > maxTempAlarm)
    {
        if (highOutStart == 0) // перший вихід
        {
            highOutStart = now;
            Serial.println("Temperature above max started");
        }
        else
        {
            // додаємо час у виході
            totalHighOutTime += (now - highOutStart);
            highOutStart = now;

            if (!isHighAlarm && (totalHighOutTime >= HIGH_ALARM_TIME))
            {
                isHighAlarm = true;
                Serial.println("HIGH ALARM: temp above max for >15 minutes");
            }
        }
    }
    else
    {
        highOutStart = 0;
    }
}

void updateTemp()
{
    if (millis() - currTime >= UPDATE_TIME)
    {
        readTemp();
        if (millis() - startAlarmTimer >= DELAY_SHOW_TEMP)
        {
            checkAlarm();
        }

        if (tempC > maxTemp)
        {
            maxTemp = tempC;
        }
        Serial.print("Max Temperature: ");
        Serial.println(maxTemp);

        if (tempC < minTemp)
        {
            minTemp = tempC;
        }
        Serial.print("Min Temperature: ");
        Serial.println(minTemp);

        printEPaper();
        currTime = millis();

        hourSum += tempC;
        hourCount++;
    }

    // раз на годину
    if (millis() - lastHourTime >= HOUR_TIME)
    {
        if (hourCount > 0)
        {
            float hourAvg = hourSum / hourCount;
            daySum += hourAvg;
            hoursPassed++;

            Serial.print("Hour average: ");
            Serial.println(hourAvg);
        }

        // обнулити для наступної години
        hourSum = 0;
        hourCount = 0;
        lastHourTime = millis();
    }

    // раз на добу
    if (millis() - lastDayTime >= DAY_TIME)
    {
        if (hoursPassed > 0)
        {
            avrTemp = daySum / hoursPassed;
            Serial.print("Daily average temperature: ");
            Serial.println(avrTemp);
        }

        logToJson();

        // --- скидати мін/макс/середнє ---
        maxTemp = -1000.0;
        minTemp = 1000.0;
        avrTemp = 0;

        // обнулити для наступної доби
        daySum = 0;
        hoursPassed = 0;
        lastDayTime = millis();

        Serial.println("=== New Day: stats reset ===");
    }
}
