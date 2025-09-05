#include "config.h"

unsigned long showTempTimer = 0;

const int ledPin = LED_BUILTIN;

unsigned long waitingTime = 0;
unsigned long backToWaitTime = 0;
unsigned long enterZeroTime = 0;
unsigned long zeroSetUp = 0;

uint8_t state = 0;

bool buttonsHold = false;
bool isBtnPres = false;
bool enterHold = false;