#include <Arduino.h>
#include <Wire.h>
#include <GxEPD2_BW.h>
#define DISABLE_DIAGNOSTIC_OUTPUT
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

#define SET_BTN PB12
#define ENTER_BTN PB13
const int ledPin = LED_BUILTIN;
// Ініціалізація дисплея (2.13", SSD1680, GxEPD2)
#define EPD_CS PA4
#define EPD_DC PB0
#define EPD_RST PB1
#define EPD_BUSY PB10

#define SENSOR_ADDR 0x48

#define WAITING_MODE_DELAY 3000
#define BACK_TO_WAIT 60000
#define ENTER_TIME 6000

#define UPDATE_TIME 2000
// #define RELAX_TIME 1800000UL
#define RELAX_TIME 30000

unsigned long waitingTime = 0;
unsigned long backToWaitTime = 0;
unsigned long enterZeroTime = 0;
unsigned long zeroSetUp = 0;

uint8_t state = 0;

float tempC = 0;
float maxTemp = -1000.0;
float minTemp = 1000.0;
float maxTempAlarm = 30;
float minTempAlarm = 20;

bool isAlarm = false;
bool isOutOfRange = false;

void handleButtons();
void stateMachine(uint8_t &state);
void printDate();
void incrementField();
void handleData();
void setDataEpaper();
void showTemperature();

uint8_t day = 1;
uint8_t month = 1;
uint16_t year = 2025;
uint8_t hour = 0;
uint8_t minute = 0;

uint8_t currentField = 0;

bool lastSetState = HIGH;
bool lastEnterState = HIGH;

bool buttonsHold = false;
bool isBtnPres = false;
bool enterHold = false;

GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(GxEPD2_213_BN(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

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

    display.init(115200);
    display.setRotation(3);
    display.setFont(&FreeSansBold12pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setPartialWindow(0, 0, display.width(), display.height());
    printEPaper();
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
    if (enter_Btn_State == LOW && set_Btn_State == HIGH)
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
}

void stateMachine(uint8_t &state)
{
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

    switch (state)
    {
    case 1:
        handleData();
        // setDataEpaper();

        break;

    case 2:
        showTemperature();
        break;

    default:
        break;
    }
}

void handleData()
{
    bool setState = digitalRead(SET_BTN);
    bool enterState = digitalRead(ENTER_BTN);

    // натиснута SET (LOW, бо INPUT_PULLUP)
    if (setState == LOW && lastSetState == HIGH)
    {
        incrementField();
        printDate();
        setDataEpaper();
        delay(200); // антидребезг
    }

    // натиснута ENTER
    if (enterState == LOW && lastEnterState == HIGH)
    {
        currentField = (currentField + 1) % 5; // переходимо до наступного поля
        Serial.print("Selected field: ");
        Serial.println(currentField);
        setDataEpaper();

        delay(200);
    }

    lastSetState = setState;
    lastEnterState = enterState;
}

void incrementField()
{
    switch (currentField)
    {
    case 0: // день
        day++;
        if (day > 31)
            day = 1;
        break;

    case 1: // місяць
        month++;
        if (month > 12)
            month = 1;
        break;

    case 2: // рік
        year++;
        if (year > 2100)
            year = 2000;
        break;

    case 3: // години
        hour++;
        if (hour > 23)
            hour = 0;
        break;

    case 4: // хвилини
        minute++;
        if (minute > 59)
            minute = 0;
        break;
    }
}

void printDate()
{
    Serial.print("Date: ");
    Serial.print(day);
    Serial.print(".");
    Serial.print(month);
    Serial.print(".");
    Serial.print(year);

    Serial.print(" Time: ");
    Serial.print(hour);
    Serial.print(":");
    Serial.println(minute);
}

void showTemperature()
{
    // приклад: просто друк у Serial
    // Serial.println("Temperature: 25 C");
}
void printEPaper() {}

void setDataEpaper()
{

    char buffer1[32], buffer2[32];

    snprintf(buffer1, sizeof(buffer1), "%02d.%02d.%04d", day, month, year);
    snprintf(buffer2, sizeof(buffer2), "%02d:%02d", hour, minute);

    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);

        display.setFont(&FreeSans12pt7b);
        display.setCursor(10, 30);
        display.print(buffer1);

        display.setFont(&FreeSans12pt7b);
        display.setCursor(150, 120);
        display.print(buffer2);
    } while (display.nextPage());
}
