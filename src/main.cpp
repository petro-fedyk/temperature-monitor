#include <Arduino.h>
#include <Wire.h>
#include <GxEPD2_BW.h>
#include <STM32RTC.h>
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
// #define WAITING_MODE_DELAY 10000
#define BACK_TO_WAIT 60000
#define ENTER_TIME 6000
// #define DELAY_SHOW_TEMP 1200000
#define DELAY_SHOW_TEMP 5000

#define UPDATE_TIME 2000
// #define RELAX_TIME 1800000UL
#define RELAX_TIME 30000

unsigned long waitingTime = 0;
unsigned long backToWaitTime = 0;
unsigned long enterZeroTime = 0;
unsigned long zeroSetUp = 0;
unsigned long showTempTimer = 0;

unsigned long currTime = 0;
unsigned long relaxCurr = 0;

uint8_t state = 0;

float tempC = 0;
float maxTemp = -1000.0;
float minTemp = 1000.0;
float maxTempAlarm = 30;
float minTempAlarm = 20;

bool isAlarm = false;
bool isOutOfRange = false;

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
        updateTemp();
        // setDateRTC();

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
            year = 2025;
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

void setDateRTC()
{
    uint8_t second = 0;

    rtc.setHours(hour);
    rtc.setMinutes(minute);
    rtc.setDay(day);
    rtc.setMonth(month);
    rtc.setYear(year - 2000);
}

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

void printEPaper()
{
    char buffer1[32], buffer2[32], buffer3[32], buffer4[32], buffer5[32], buffer6[32],
        buffer7[32], buffer8[32], buffer9[32], buffer10[32], buffer11[32], buffer12[32];

    // Температура у сотих (наприклад, 24.96 °C → 2496)
    int tempInt = (int)(tempC * 100);
    int maxInt = (int)(maxTemp * 100);
    int minInt = (int)(minTemp * 100);
    int avgInt = (int)(maxTemp * 100); // (тимчасово, якщо треба буде — перерахуємо справжнє середнє)

    // Форматування рядків
    snprintf(buffer1, sizeof(buffer1), "%d.%02d C", tempInt / 100, abs(tempInt % 100));
    snprintf(buffer2, sizeof(buffer2), "Max: %d.%02d C", maxInt / 100, abs(maxInt % 100));
    snprintf(buffer3, sizeof(buffer3), "Min: %d.%02d C", minInt / 100, abs(minInt % 100));
    snprintf(buffer4, sizeof(buffer4), "Avg: %d.%02d C", avgInt / 100, abs(avgInt % 100));

    // snprintf(buffer5, sizeof(buffer5), "%02d.%02d.%04d", day, month, year);
    // snprintf(buffer6, sizeof(buffer6), "%02d:%02d", hour, minute);

    snprintf(buffer5, sizeof(buffer5), "%02d.%02d.%04d", rtc.getDay(), rtc.getMonth(), rtc.getYear() + 2000);
    snprintf(buffer6, sizeof(buffer6), "%02d:%02d", rtc.getHours(), rtc.getMinutes());

    snprintf(buffer7, sizeof(buffer7), "Current date");
    snprintf(buffer8, sizeof(buffer8), "Current Temp");
    snprintf(buffer9, sizeof(buffer9), "Last");
    snprintf(buffer10, sizeof(buffer10), "Update");

    display.setPartialWindow(0, 0, display.width(), display.height());
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);

        // current date
        display.setFont(&FreeMono9pt7b);
        display.setCursor(5, 10);
        display.print(buffer7);

        // date
        display.setFont(&FreeSans12pt7b);
        display.setCursor(10, 30);
        display.print(buffer5);

        // current temperature
        display.setFont(&FreeMono9pt7b);
        display.setCursor(5, 45);
        display.print(buffer8);

        if (millis() - showTempTimer >= DELAY_SHOW_TEMP)
        {

            // Термометр + температура
            display.setFont(&FreeSansBold12pt7b);

            drawThermometerSymbol(5, 50);
            display.setCursor(25, 70);
            display.print(buffer1);

            display.setFont(&FreeMono9pt7b);

            display.setCursor(10, 87);
            display.print(buffer2);
            display.setCursor(10, 102);
            display.print(buffer3);
            display.setCursor(10, 115);
            display.print(buffer4);

            // Галочка/хрестик
            if (isAlarm)
            {
                drawCrossMark(display.width() - 50, 15);
            }
            else
            {
                drawCheckMark(display.width() - 50, 15);
            }
        }

        // last update
        display.setFont(&FreeMono9pt7b);
        display.setCursor(160, 85);
        display.print(buffer9);

        display.setFont(&FreeMono9pt7b);
        display.setCursor(160, 100);
        display.print(buffer10);

        display.setFont(&FreeSans12pt7b);
        display.setCursor(180, 120);
        display.print(buffer6);

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
