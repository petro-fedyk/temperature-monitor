#ifndef CONFIG_H
#define CONFIG_H

#define SET_BTN PB12
#define ENTER_BTN PB13

// Ініціалізація дисплея (2.13", SSD1680, GxEPD2)
#define EPD_CS PA4
#define EPD_DC PB0
#define EPD_RST PB1
#define EPD_BUSY PB10

// #define DELAY_SHOW_TEMP 1200000
#define DELAY_SHOW_TEMP 1000

extern unsigned long showTempTimer;

#endif