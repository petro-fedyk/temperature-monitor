#include <Arduino.h>
#include <Wire.h>
#include <STM32RTC.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

#include "temperature.h"
#include "date.h"
#include "config.h"
#include "epaper.h"

const int ledPin = LED_BUILTIN;

#define WAITING_MODE_DELAY 3000
// #define WAITING_MODE_DELAY 10000
#define BACK_TO_WAIT 60000
#define ENTER_TIME 6000

unsigned long waitingTime = 0;
unsigned long backToWaitTime = 0;
unsigned long enterZeroTime = 0;
unsigned long zeroSetUp = 0;

uint8_t state = 0;

bool buttonsHold = false;
bool isBtnPres = false;
bool enterHold = false;

GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(GxEPD2_213_BN(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));
STM32RTC &rtc = STM32RTC::getInstance();

void handleButtons();
void stateMachine(uint8_t &state);
void printDate();
void incrementField();
void handleData();
void setDataEpaper();
void setDateRTC();

void readTemp();
uint16_t readRegister16(uint8_t address, uint8_t reg);
void checkAlarm();
void updateTemp();
void printEPaper();
void drawCheckMark(int16_t x_offset, int16_t y_offset);
void drawCrossMark(int16_t x_offset, int16_t y_offset);
void drawThermometerSymbol(int16_t x, int16_t y);

void setup()
{
    Serial.begin(115200);
    pinMode(SET_BTN, INPUT_PULLUP); // <-- підтяжка вбудована
    pinMode(ENTER_BTN, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);

    Wire.begin();

    rtc.begin();

    display.init(115200);
    display.setRotation(3);
    display.setFont(&FreeSansBold12pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setPartialWindow(0, 0, display.width(), display.height());
    Serial.println("SetUp complete");
}

void loop()
{
    handleButtons();
    stateMachine(state);
}

void handleButtons()
{
    uint8_t set_Btn_State = digitalRead(SET_BTN);
    uint8_t enter_Btn_State = digitalRead(ENTER_BTN);

    // Якщо обидві кнопки натиснуті
    if (set_Btn_State == LOW && enter_Btn_State == LOW)
    {
        if (!buttonsHold)
        {
            waitingTime = millis();
            buttonsHold = true;
            Serial.println("Press both buttons...");
        }
        if (millis() - waitingTime >= WAITING_MODE_DELAY)
        {
            state = 1; // Режим налаштування дати
            Serial.println("→ State 1 (setup date)");
        }
    }
    else
    {
        buttonsHold = false;
    }

    // Якщо натиснута лише ENTER
    if (enter_Btn_State == LOW && set_Btn_State == HIGH && state == 1)
    {
        if (!enterHold)
        {
            enterZeroTime = millis();
            enterHold = true;
        }
        if (millis() - enterZeroTime >= ENTER_TIME)
        {
            state = 2; // Режим перегляду температури
            Serial.println("→ State 2 (temperature)");
            setDateRTC();
            showTempTimer = millis();
        }
    }
    else
    {
        enterHold = false; // скидати, якщо ENTER відпущено
    }

    // Якщо будь-яка кнопка натиснута – оновлюємо таймер "засинання"
    if (set_Btn_State == LOW || enter_Btn_State == LOW)
    {
        isBtnPres = true;
        backToWaitTime = millis();
    }
    else
    {
        isBtnPres = false;
    }

    if (state == 1 && !isBtnPres)
    {
        if (millis() - backToWaitTime >= BACK_TO_WAIT)
        {
            state = 0;
            Serial.println("Back to state 0");
            Serial.print(state);
            Serial.println(" state");
        }
    }
}

void stateMachine(uint8_t &state)
{

    switch (state)
    {
    case 1:
        handleData();
        // setDataEpaper();

        break;

    case 2:
        updateTemp();
        // setDateRTC();

        break;

    default:
        break;
    }
}
