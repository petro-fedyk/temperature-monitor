#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>
#define SET_BTN PB12
#define ENTER_BTN PB13

// Ініціалізація дисплея (2.13", SSD1680, GxEPD2)
#define EPD_CS PA4
#define EPD_DC PB0
#define EPD_RST PB1
#define EPD_BUSY PB10

// #define DELAY_SHOW_TEMP 1200000
#define DELAY_SHOW_TEMP 10000
#define DELAY_SHOW_ALARM 10000

#define WAITING_MODE_DELAY 3000
// #define WAITING_MODE_DELAY 10000
#define BACK_TO_WAIT 60000
#define ENTER_TIME 6000

extern const int ledPin;

extern unsigned long waitingTime;
extern unsigned long backToWaitTime;
extern unsigned long enterZeroTime;
extern unsigned long zeroSetUp;

extern uint8_t state;

extern bool buttonsHold;
extern bool isBtnPres;
extern bool enterHold;

extern unsigned long showTempTimer;
extern unsigned long showAlarmTimer;

#endif