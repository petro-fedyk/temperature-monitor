#include <Arduino.h>
#include <Wire.h>
#include "Protocentral_MAX30205.h"
#include <GxEPD2_BW.h>
#include <Fonts/FreeSansBold12pt7b.h>

#define BTN_PIN 12
bool isBtnPress = false;

#define UPDATE_TIME 2000
// #define RELAX_TIME 1800000UL
#define RELAX_TIME 30000
#define HOLD_TIME 500

unsigned long currTime = 0;
unsigned long relaxCurr = 0;
unsigned long holdTime = 0;

float tempC = 0;
float maxTemp = -1000.0;
float minTemp = 1000.0;
float maxTempAlarm = 30;
float minTempAlarm = 20;

bool isAlarm = false;
bool isOutOfRange = false;

MAX30205 tempSensor;

GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(GxEPD2_213_BN(15, 4, 5, 16));

void startBtn();
void readTemp();
void checkAlarm();
void checkAlarm();
void updateTemp();
void printEPaper();
void drawCheckMark(int16_t x_offset, int16_t y_offset);
void drawCrossMark(int16_t x_offset, int16_t y_offset);
void drawThermometerSymbol(int16_t x, int16_t y);

void setup()
{

  Serial.begin(115200);

  pinMode(BTN_PIN, INPUT);
  Wire.begin(0, 2);

  tempSensor.sensorAddress = 0x4F;

  Serial.println("Starting MAX30205 sensor...");
  tempSensor.begin();

  if (!tempSensor.scanAvailableSensors())
  {
    Serial.println("MAX30205 not found. Check wiring.");
  }
  else
  {
    Serial.println("Sensor initialized successfully.");
  }

  display.init(115200);
  display.setRotation(3);
  display.setFont(&FreeSansBold12pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setPartialWindow(0, 0, display.width(), display.height());
  printEPaper();
}

void loop()
{
  startBtn();
  if (isBtnPress)
  {

    updateTemp();
  }
  else
  {
    Serial.println("device in waiting mode");
  }
}

void startBtn()
{
  static bool btnPreviouslyPressed = false;
  static unsigned long btnPressTime = 0;

  bool btnState = digitalRead(BTN_PIN) == LOW; // Кнопка натиснута, якщо LOW

  // Якщо пристрій вже активовано, не перевіряємо кнопку
  if (isBtnPress)
    return;

  if (btnState && !btnPreviouslyPressed)
  {
    btnPressTime = millis();
  }

  if (btnState && (millis() - btnPressTime >= HOLD_TIME))
  {
    isBtnPress = true;
    Serial.println("device is activate");
  }

  btnPreviouslyPressed = btnState;
}

void readTemp()
{
  tempC = tempSensor.getTemperature();
  Serial.print("Temperature: ");
  Serial.print(tempC);
  Serial.println(" °C");
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

void printEPaper()
{
  char buffer1[32], buffer2[32], buffer3[32];

  snprintf(buffer1, sizeof(buffer1), "Temp %.1f °C", tempC);   // Температура
  snprintf(buffer2, sizeof(buffer2), "Max: %.1f °C", maxTemp); // Макс
  snprintf(buffer3, sizeof(buffer3), "Min: %.1f °C", minTemp); // Мін

  display.setPartialWindow(0, 0, display.width(), display.height());
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    // Термометр + температура
    drawThermometerSymbol(10, 10);
    display.setCursor(40, 30); // Текст справа від термометра
    display.print(buffer1);

    // Макс/мін
    display.setCursor(10, 70);
    display.print(buffer2);
    display.setCursor(10, 110);
    display.print(buffer3);

    // Галочка/хрестик
    if (isAlarm)
    {
      drawCrossMark(display.width() - 50, 15);
    }
    else
    {
      drawCheckMark(display.width() - 50, 15);
    }

  } while (display.nextPage());
}

void drawCheckMark(int16_t x, int16_t y)
{
  // Велика жирна галочка (6 паралельних ліній)
  for (int i = 0; i < 6; i++)
  {
    display.drawLine(x + i, y + 20, x + 10 + i, y + 35, GxEPD_BLACK);
    display.drawLine(x + 10 + i, y + 35, x + 35 + i, y + 0, GxEPD_BLACK);
  }
}

void drawCrossMark(int16_t x, int16_t y)
{
  // Великий жирний хрестик (6 паралельних діагоналей)
  for (int i = 0; i < 6; i++)
  {
    display.drawLine(x + i, y, x + 35 + i, y + 35, GxEPD_BLACK);
    display.drawLine(x + i, y + 35, x + 35 + i, y, GxEPD_BLACK);
  }
}

void drawThermometerSymbol(int16_t x, int16_t y)
{
  // Зовнішній контур (лівий і правий)
  display.drawLine(x + 3, y + 0, x + 3, y + 20, GxEPD_BLACK);
  display.drawLine(x + 9, y + 0, x + 9, y + 20, GxEPD_BLACK);

  // Верхній заокруглений контур
  display.drawCircle(x + 6, y + 0, 3, GxEPD_BLACK);

  // Мітки (риски температури)
  display.drawLine(x + 9, y + 4, x + 13, y + 4, GxEPD_BLACK);
  display.drawLine(x + 9, y + 8, x + 13, y + 8, GxEPD_BLACK);
  display.drawLine(x + 9, y + 12, x + 13, y + 12, GxEPD_BLACK);

  // Вертикальна трубка з ртуттю
  display.fillRect(x + 5, y + 10, 2, 10, GxEPD_BLACK);

  // Нижня кулька (резервуар)
  display.fillCircle(x + 6, y + 22, 5, GxEPD_BLACK);

  // Зовнішній контур нижньої кульки
  display.drawCircle(x + 6, y + 22, 6, GxEPD_BLACK);
}
