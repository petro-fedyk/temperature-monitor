#include "date.h"

uint8_t day = 1;
uint8_t month = 1;
uint16_t year = 2025;
uint8_t hour = 0;
uint8_t minute = 0;

uint8_t currentField = 0;
bool lastSetState = HIGH;
bool lastEnterState = HIGH;

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

void setDateRTC()
{
    uint8_t second = 0;

    rtc.setHours(hour);
    rtc.setMinutes(minute);
    rtc.setDay(day);
    rtc.setMonth(month);
    rtc.setYear(year - 2000);
}