#ifndef EPAPER_H
#define EPAPER_H
#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

#include "config.h"
#include "date.h"
#include "temperature.h"

extern GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display;
void setDataEpaper();
void printEPaper();
void dateEpaper();
void drawCheckMark(int16_t x_offset, int16_t y_offset);
void drawCrossMark(int16_t x_offset, int16_t y_offset);
void drawThermometerSymbol(int16_t x, int16_t y);
#endif