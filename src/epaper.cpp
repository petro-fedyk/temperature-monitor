#include "epaper.h"

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

void printEPaper()
{
    char buffer1[32], buffer2[32], buffer3[32], buffer4[32], buffer5[32], buffer6[32],
        buffer7[32], buffer8[32], buffer9[32], buffer10[32], buffer11[32], buffer12[32];

    // Температура у сотих (наприклад, 24.96 °C → 2496)
    int tempInt = (int)(tempC * 100);
    int maxInt = (int)(maxTemp * 100);
    int minInt = (int)(minTemp * 100);
    int avgInt = (int)(avrTemp * 100); // (тимчасово, якщо треба буде — перерахуємо справжнє середнє)

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
