
#include <P1AM.h>
#include <math.h>

int modInput = 1;
int modOutput = 2;
int modAnalogIn = 3;

int pulseKey = 1;
int pinLB1 = 2;
int pinLB2 = 3;
int pinLBW = 4;
int pinLBR = 5;
int pinLBB = 6;

int linkageDistances[] = {3, 7, 12};
int valvePins[] = {3, 4, 5};

enum MachineStates {
  REST, 
  SENSE,    
  TRACKER,  
  ACTUATE,
  COUNT
};

enum Colors {
  WHITE,
  RED,
  BLUE,
  COLORCOUNT
};

Colors targetColor = WHITE;

MachineStates mState = REST;

void setup(){ 
  Serial.begin(115200);  //initialize serial communication at 115200 bits per second 
  while (!P1.init()){   
  }
}

void TurnEverythingOff() {
  for (int i = 1; i < 6; i++) { 
    P1.writeDiscrete(LOW, modOutput, i);
  }	
}

void ToggleConveyor(bool onOffState) {
  P1.writeDiscrete(onOffState, modOutput, 1);
}

void ToggleCompressor(bool onOffState) {
  P1.writeDiscrete(onOffState, modOutput, 2);
}

void UpdateRobotArmOutputs() {
  for (int i = 4; i < 7; i++) {
    P1.writeDiscrete(!P1.readDiscrete(modInput, i), modOutput, i + 2);
  }
  
}

int channelTwo;
int color = 0;
int linkageCount = 0;
bool prevKeyState = false;
bool currentState = false;
int distanceToMove = 8;
int defWrongColor = 10000;
int currentColor = 10000;

void loop(){  
  UpdateRobotArmOutputs();
  switch (mState)
  {
  case MachineStates::REST:
    TurnEverythingOff();
    //Serial.print("Rest state");
    // Check to see if LB 1 is broken
    if (!P1.readDiscrete(modInput, pinLB1)) {
      //Serial.print("Switching state");
      // Switch states
      mState = MachineStates::SENSE;
    }
    break;
  case MachineStates::SENSE: 
    currentColor = min(currentColor, P1.readAnalog(modAnalogIn, 1));
    //Serial.print("Sense state, min color: ");
    //Serial.println(currentColor);
    ToggleConveyor(HIGH);
    // Wait for LB 2 to be triggered
    if (!P1.readDiscrete(modInput, pinLB2)) {
      //Serial.print("Switching state");
      mState = MachineStates::TRACKER;
      if (currentColor < 3000) {
        targetColor = Colors::WHITE;
      } else if (currentColor < 4800) {
        targetColor = Colors::RED;
      } else {
        targetColor = Colors::BLUE;
      }
    }
    break;
  case MachineStates::TRACKER:
    //Serial.print("Tracker state");
    ToggleCompressor(HIGH);
    currentState = (bool)P1.readDiscrete(modInput, pulseKey);
    if (!prevKeyState && currentState) {
      linkageCount++;
    }
    prevKeyState = currentState;
    if (linkageCount > linkageDistances[(int)targetColor]) {
      //Serial.print("Switching state");
      mState = MachineStates::ACTUATE;
    }
    break;
  case MachineStates::ACTUATE:
    //Serial.print("Actuate state");
    ToggleConveyor(LOW);
    P1.writeDiscrete(HIGH, modOutput, valvePins[(int)targetColor]);
    delay(1000);
    mState = MachineStates::REST;
    currentColor = defWrongColor;
    linkageCount = 0;
    break;
  case MachineStates::COUNT:
    break; 
  }

  
}