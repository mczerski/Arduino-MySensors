/*
  TimedWakeup

  This sketch demonstrates the usage of Internal Interrupts to wakeup a chip
  in deep sleep mode.

  In this sketch, the internal RTC will wake up the processor every second.

  This example code is in the public domain.
*/

#include "STM32LowPower.h"

void setup() {
  // Configure low power
  LowPower.begin();
}

void blink()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(LED_BUILTIN, INPUT_FLOATING);  
  delay(500);
}

void loop() {
  blink();
  LowPower.deepSleep(10000);
  blink();
  blink();
  LowPower.shutdown(10000);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}
