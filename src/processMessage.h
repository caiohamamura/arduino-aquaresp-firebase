#include <Arduino.h>
#include <defines.h>
#include <Regexp.h>
#include <stdlib.h>

const char *MESSAGE_PATTERN = ".*gpio([0-9]+) (-?[0-9]+)";
const char *PATTERN_LAST_TWO = "([^ ]+) ([0-9]+)$";

void processMessage(char *message)
{
  MatchState ms(message);
  ms.Match(MESSAGE_PATTERN);
  char cap[10];

  if (ms.level > 1)
  {
    ms.GetCapture(cap, 0);
    int matchedGpioNumber = atoi(cap);
    Serial.printf("Matched gpio: %d\n", matchedGpioNumber);

    ms.GetCapture(cap, 1);
    int value = atoi(cap);
    Serial.printf("Matched value: %d\n", value);

    if (value >= 0)
    {
      if (matchedGpioNumber == LED_PIN)
      {
        autoLED = false;
        ledPwm = value;
      }
      analogWrite(matchedGpioNumber, value);
    }
    else if (matchedGpioNumber == LED_PIN)
    {
      autoLED = true;
    }
  }
  else
  {
    ms.Match(PATTERN_LAST_TWO);
    if (ms.level > 1)
    {
      Serial.println("Outro padrao: " + String(message));
      ms.GetCapture(cap, 0);
      if (strcmp(cap, "hourBegin") == 0)
      {
        ms.GetCapture(cap, 1);
        int hour = atoi(cap);
        if (hour >= 0 && hour < 24)
        {
          hourBegin = hour;
          Serial.printf("New hour begin: %d\n", hourBegin);
        }
      }
      else if (strcmp(cap, "hourEnd") == 0)
      {
        ms.GetCapture(cap, 1);
        int hour = atoi(cap);
        if (hour > 0 && hour <= 24)
        {
          hourEnd = hour;
          Serial.printf("New hour end: %d\n", hourEnd);
        }
      }
    }
  }
}
