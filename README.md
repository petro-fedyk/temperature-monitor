# STM32 ePaper Temperature and Date Display

This repository contains an STM32-based project that displays **temperature and date** on a **GxEPD2 ePaper display**. It uses an RTC module to keep track of the date and time and supports basic user interaction with two buttons.

---

## Features

- Reads temperature from a sensor (via I2C).
- Displays date and temperature on a **2.13" ePaper display**.
- Simple **two-button interface**:
  - **SET button** for entering setup modes.
  - **ENTER button** for confirming actions.
- Partial screen updates for fast display refresh.
- Uses STM32 **RTC** for accurate timekeeping.
- Debug output over UART at **115200 baud**.

---

## Hardware Requirements

- STM32 microcontroller (any STM32 compatible with Wire/I2C and UART).
- ePaper display (GxEPD2 library compatible, 2.13" used here).
- RTC module (STM32 internal RTC used via STM32RTC library).
- Two push-buttons (SET and ENTER).
- I2C wiring for temperature sensor.
- UART connection for debugging (optional).

---

## Wiring (example)

| STM32 Pin        | Function                  |
|-----------------|---------------------------|
| I2C SDA / SCL     | Temperature sensor       |
| SET button        | Digital input            |
| ENTER button      | Digital input            |
| EPD_CS, EPD_DC    | ePaper SPI               |
| EPD_RST, EPD_BUSY | ePaper control signals   |
| UART TX/RX        | Debug output (optional)  |

---

## Software Requirements

- **Arduino IDE** or compatible environment.
- STM32 core for Arduino installed.
- Libraries:
  - `Wire`
  - `STM32RTC`
  - `GxEPD2`
  - `Fonts` for GxEPD2 display.

---

## Usage

1. Open the project in **Arduino IDE**.
2. Select the correct STM32 board.
3. Flash the firmware to your STM32 board.
4. Open Serial Monitor at **115200 baud** to see debug output.
5. Press **SET + ENTER** to enter setup mode for date.
6. Press **ENTER** to confirm and switch to temperature display.

---

## Code Overview

- **main.ino**: handles buttons, state machine, ePaper updates, and temperature reading.
- **temperature.h/cpp**: routines for reading temperature via I2C.
- **date.h/cpp**: date handling and RTC configuration.
- **epaper.h/cpp**: ePaper drawing and partial screen update functions.
- **config.h**: pin definitions and constants for button timing and delays.

---

## License

This project is provided **as-is**. See the LICENSE file for details.
