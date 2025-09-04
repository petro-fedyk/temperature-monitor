#ifndef DATE_H
#define DATE_H
#include <Arduino.h>
#include <STM32RTC.h>
#include "config.h"

extern uint8_t day;
extern uint8_t month;
extern uint16_t year;
extern uint8_t hour;
extern uint8_t minute;

extern uint8_t currentField;
extern STM32RTC &rtc;

void handleData();
void printDate();
void incrementField();
void setDataEpaper();
#endif