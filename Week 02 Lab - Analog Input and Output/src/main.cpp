#include <Arduino.h>
#include <string.h>
#include <Ticker.h>
 
String printSentence = "";
volatile bool isSentenceComplete = false;
long prevSampleTime = 0;
long timeBetweenSamplesMs = 100;

void myFunction();

Ticker newTimerFn(myFunction, timeBetweenSamplesMs, 0, MILLIS);

void setup() {
  Serial.begin(9600);
  delay(2000);
  
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLDOWN);
  pinMode(2, INPUT_PULLDOWN);

  newTimerFn.start();
} 

void loop() {
  long currentTime = millis();
  newTimerFn.update();
  if (isSentenceComplete) {
    Serial.println(printSentence);
    isSentenceComplete = false;
  }
}

void myFunction() {
  printSentence = "D0, D1, D2, A0: ";
  printSentence += (String(digitalRead(0)) + ", " + String(digitalRead(1)) + " , " + String(digitalRead(2)));
  printSentence += " , " + String(analogRead(A0)); 
  isSentenceComplete = true;
} 