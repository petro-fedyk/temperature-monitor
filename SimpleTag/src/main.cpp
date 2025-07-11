#include <Arduino.h>
#include <Wire.h>
#include "Protocentral_MAX30205.h"

#define UPDATE_TIME 2000

#define GREEN_ZONE_MAX 20
#define YELLOW_ZONE_MAX 30
#define RED_ZONE_MAX 35

#define GREEN_ZONE_MIN 15
#define YELLOW_ZONE_MIN 10
#define RED_ZONE_MIN 5

#define GREEN_PIN 15
#define YELLOW_PIN 13
#define RED_PIN 17

#define BTN_PIN 12

bool isBtnPress = false;

// #define STATE_HOLD_TIME 1800000UL
#define STATE_HOLD_TIME 5000
#define HOLD_TIME 500

uint8_t state = 1;

unsigned long currTime = 0;

float maxTemperature = -1000.0;
float minTemperature = 1000.0;

float tempC = 0;

uint8_t lastState = 1;
uint8_t candidateState = 1;
uint8_t finalyState = 1;
unsigned long candidateSince = 0;
unsigned long holdTime = 0;

MAX30205 tempSensor;

void startBtn()
{
  static bool btnPreviouslyPressed = false;
  static unsigned long btnPressTime = 0;

  bool btnState = digitalRead(BTN_PIN) == LOW; // LOW = натиснута (якщо INPUT_PULLUP)

  if (btnState && !btnPreviouslyPressed)
  {
    btnPressTime = millis(); // Початок натискання
  }

  if (btnState && !isBtnPress && (millis() - btnPressTime >= HOLD_TIME))
  {
    isBtnPress = true;
    Serial.println("device is activate");
  }

  btnPreviouslyPressed = btnState;
}

void readTemp()
{
  tempC = tempSensor.getTemperature();
  Serial.print("Temperature: ");
  Serial.print(tempC);
  Serial.println(" °C");
}

void checkAlarm()
{
  if (tempC >= GREEN_ZONE_MIN && tempC <= GREEN_ZONE_MAX)
  {
    state = 1; // Зелена зона (нормальна)
  }
  else if ((tempC > GREEN_ZONE_MAX && tempC <= YELLOW_ZONE_MAX) ||
           (tempC < GREEN_ZONE_MIN && tempC >= YELLOW_ZONE_MIN))
  {
    state = 2; // Жовта зона (верхня або нижня)
  }
  else if (tempC > YELLOW_ZONE_MAX || tempC < RED_ZONE_MIN)
  {
    state = 3; // Червона зона (верхня або нижня)
  }
  else if (tempC < YELLOW_ZONE_MIN && tempC >= RED_ZONE_MIN)
  {
    state = 2; // Жовта зона (нижня частина)
  }
}

void updateState()
{
  if (state != lastState)
  {
    if (state != candidateState)
    {
      candidateState = state;
      candidateSince = millis();
    }
    else
    {
      if (millis() - candidateSince >= STATE_HOLD_TIME)
      {
        lastState = candidateState;

        if (lastState > finalyState)
        {
          finalyState = lastState;
        }
        Serial.print("State changed to ");
        Serial.println(finalyState);
      }
    }
  }
  else
  {
    // Температура знову стабілізувалась — скидаємо кандидата
    candidateState = lastState;
    candidateSince = millis();
  }
}
void showState(uint8_t state)
{

  switch (state)
  {
  case 1:
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(RED_PIN, LOW);
    Serial.println("green zone");
    break;
  case 2:
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(YELLOW_PIN, HIGH);
    digitalWrite(RED_PIN, LOW);
    Serial.println("yellow zone");
    break;
  case 3:
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(RED_PIN, HIGH);
    Serial.println("red zone");
    break;
  }
}

void updateTemp()
{
  if (millis() - currTime >= UPDATE_TIME)
  {
    readTemp();
    checkAlarm();
    showState(finalyState);

    currTime = millis();
  }
}

void setup()
{

  Serial.begin(115200);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT);
  Wire.begin(0, 2);

  tempSensor.sensorAddress = 0x4F;

  Serial.println("Starting MAX30205 sensor...");
  tempSensor.begin();

  if (!tempSensor.scanAvailableSensors())
  {
    Serial.println("MAX30205 not found. Check wiring.");
  }
  else
  {
    Serial.println("Sensor initialized successfully.");
  }
}

void loop()
{
  startBtn();
  if (isBtnPress)
  {

    updateTemp();
    updateState();
  }
  else
  {
    Serial.println("device in waiting mode");
  }
}
