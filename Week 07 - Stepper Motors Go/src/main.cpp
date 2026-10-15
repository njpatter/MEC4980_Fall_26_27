#include <Arduino.h>
#include <AccelStepper.h>

int pinStep = 9;
int pinDir = 10;

AccelStepper onlyAxis(1, pinStep, pinDir);

void setup() {
  pinMode(pinStep, OUTPUT);
  pinMode(pinDir, OUTPUT);

  onlyAxis.setMaxSpeed(10000);
  onlyAxis.setSpeed(300);
  onlyAxis.setAcceleration(10000);
}

void loop() {
  onlyAxis.runToNewPosition(200);
  delay(1000);
  onlyAxis.runToNewPosition(-200);
  onlyAxis.stop();
  delay(500);
}

