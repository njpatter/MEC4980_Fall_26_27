#include <Arduino.h>

const int PIN1 = 9;
const int PIN2 = 10;
const int PWM_FREQ = 20000;
const int PWM_BITS = 8;

void setup() {
  ledcAttach(PIN1, PWM_FREQ, PWM_BITS);
  ledcAttach(PIN2, PWM_FREQ, PWM_BITS);
}

void loop() {
  ledcWrite(PIN1, 128);
  ledcWrite(PIN2, 0);
}

