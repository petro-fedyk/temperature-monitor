#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <Arduino.h>
#include "config.h"

extern float tempC;
extern float maxTemp;
extern float minTemp;
extern float avrTemp;
extern float maxTempAlarm;
extern float minTempAlarm;

extern bool isLowAlarm;
extern bool isHighAlarm;

extern unsigned long lowOutStart;
extern unsigned long highOutStart;

void readTemp();
uint16_t readRegister16(uint8_t address, uint8_t reg);
void checkAlarm();
void updateTemp();
void printEPaper();

#endif
