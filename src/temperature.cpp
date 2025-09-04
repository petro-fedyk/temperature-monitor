#include "temperature.h"
#include <Wire.h>

// глобальні змінні тут
float tempC = 0;
float maxTemp = -1000.0;
float minTemp = 1000.0;
float maxTempAlarm = 30;
float minTempAlarm = 20;

bool isAlarm = false;
bool isOutOfRange = false;

unsigned long currTime = 0;
unsigned long relaxCurr = 0;

#define SENSOR_ADDR 0x48
#define UPDATE_TIME 2000
#define RELAX_TIME 30000

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
    bool tempOutOfRange = (tempC > maxTempAlarm || tempC < minTempAlarm);

    if (!isAlarm) // Якщо тривога ще не була активована
    {
        if (tempOutOfRange)
        {
            if (!isOutOfRange)
            {
                // Перший вихід за межі — старт відліку
                isOutOfRange = true;
                relaxCurr = millis();
                Serial.println("Out of range started");
            }
            else if (millis() - relaxCurr >= RELAX_TIME)
            {
                // Якщо тривалість перевищила RELAX_TIME — активуємо тривогу
                isAlarm = true;
                Serial.println("Alarm is activate");
            }
        }
        else
        {
            // Температура повернулась в межі до завершення RELAX_TIME
            isOutOfRange = false;
        }
    }
}

void updateTemp()
{
    if (millis() - currTime >= UPDATE_TIME)
    {
        readTemp();
        checkAlarm();
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
    }
}
