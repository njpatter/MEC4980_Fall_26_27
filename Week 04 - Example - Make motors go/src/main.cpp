#include <Arduino.h>

const int PIN1 = 9;
const int PIN2 = 10;
const int PWM_FREQ = 20000;
const int PWM_BITS = 8;

void setup() { 
}

void loop() {
  analogWrite(PIN1, 255);
  analogWrite(PIN2, 0);
}

