#include <Arduino.h>

bool isMainBoard = false;

String pcbuffer = "";
String boardbuffer = "";

int pinRx = 5;
int pinTx = 6;

long rateToPc = 115200;
long rateToBoard = 9600;

int DoSomeMath(int a) {
  return a * a;
}

void setup() {
  if (isMainBoard) {
    Serial.begin(rateToPc);
    while (!Serial) {
      ;
    }
  }
  Serial1.setPins(pinRx, pinTx);
  Serial1.begin(9600);

}

void RunMainBoardLoop() {
  // Not really necessary for the project
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      pcbuffer.trim();
      if (pcbuffer.length() > 0) {
        Serial1.println(pcbuffer); // Use this command to send to other board
        Serial.print(pcbuffer + " ");
        pcbuffer = "";
      }
    } else if (c != '\r') {
      pcbuffer += c;
    }
  }
  // End of non-necessary code
  // Rest of function is of questionable value
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n') {
      boardbuffer.trim();
      if (boardbuffer.length() > 0) {
        Serial.println(boardbuffer.toInt());
      }
      boardbuffer = "";
    } else if (c != '\r') {
      boardbuffer += c;
    }
  }
}

void RunSecondBoardLoop() {
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n') {
      boardbuffer.trim();
      if (boardbuffer.length() > 0) {
        //Serial1.println(boardbuffer);
        Serial1.println(DoSomeMath(boardbuffer.toInt()));
      }
      boardbuffer = "";
    } else if (c != '\r') {
      boardbuffer += c;
    }
  }
}

void loop() {
  // \r \n
  if (isMainBoard) {
    RunMainBoardLoop();
  } else {
    RunSecondBoardLoop();
  }

}
