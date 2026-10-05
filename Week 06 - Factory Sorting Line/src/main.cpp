
#include <P1AM.h>

int modInput = 1;
int modOutput = 2;
int modAnalogIn = 3;

int pulseKey = 1;
int pinLB1 = 2;
int pinLB2 = 3;
int pinLBW = 4;
int pinLBR = 5;
int pinLBB = 6;

enum MachineStates {
  REST, 
  SENSE,    
  TRACKER,  
  ACTUATE,
  COUNT
};

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

int channelTwo;
int color = 0;
void loop(){  

  switch (mState)
  {
  case MachineStates::REST:
    TurnEverythingOff();
    if (!P1.readDiscrete(modInput, pinLB1)) {
      mState = MachineStates::SENSE;
    }
    break;
  case MachineStates::SENSE:
    P1.writeDiscrete(modOutput, 1);
    break;
  
  default:
    break;
  }

  Serial.print("pulse, 1, 2, W, R, B, color: ");
  for (int i = 1; i < 7; i++) {
    channelTwo = P1.readDiscrete(modInput,i);	 
	  Serial.print(channelTwo);	
    Serial.print(", ");
  } 
  color = P1.readAnalog(modAnalogIn, 1);
	Serial.println(color);
	
  	 
  
}