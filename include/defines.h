#pragma once
#include <secret.h>

#define LED_PIN D8

// variables
volatile bool autoLED = true;
int hourBegin = 12;
int hourEnd = 19;
int ledPwm = -1;
