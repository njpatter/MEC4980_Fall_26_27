#include <Arduino.h>
#include <math.h>
#include <Adafruit_BNO08x.h>
#include <AceButton.h>
using namespace ace_button;

void setReports();
#define BNO08X_RESET -1
int pinD1 = 1;
AceButton button(pinD1);

enum AxisMode {
  MODE_BOTH, //0
  MODE_X, //1
  MODE_Y, //2 
  MODE_COUNT //3
};
AxisMode curMode = MODE_BOTH;

void ChangeMode(AceButton* button, uint8_t eventType, uint8_t buttonState) {

  // Print out a message for all events.
  Serial.print(F("handleEvent(): eventType: "));
  Serial.print(AceButton::eventName(eventType));
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);
  if (eventType == (uint8_t)AceButton::kEventDoubleClicked) {
    curMode = (AxisMode)((curMode + 1) % AxisMode::MODE_COUNT);
  }
  
}

Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  pinMode(pinD1, INPUT_PULLDOWN);
  button.init(pinD1, LOW);

  ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(ChangeMode); 
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick); 

  Serial.println("Adafruit BNO08x test!");

  // Try to initialize!
  if (!bno08x.begin_I2C()) {
    // if (!bno08x.begin_UART(&Serial1)) {  // Requires a device with > 300 byte
    // UART buffer! if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT)) {
    Serial.println("Failed to find BNO08x chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("BNO08x Found!");

  setReports();
}

void loop() {
  button.check();
  delay(10);

  if (bno08x.wasReset()) {
    Serial.print("sensor was reset ");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }

  float x = sensorValue.un.accelerometer.x;
  float y = sensorValue.un.accelerometer.y;
  float z = sensorValue.un.accelerometer.z;

  float alpha = atan2(x, sqrt(y*y + z*z)) * RAD_TO_DEG; 
  float beta = atan2(y, z) * RAD_TO_DEG;
  /*Serial.print("Accelerometer - x: ");
  Serial.print(x);
  Serial.print(" y: ");
  Serial.print(y);
  Serial.print(" z: ");
  Serial.print(z);
  Serial.print(" alpha: ");
  Serial.print(alpha);
  Serial.print(" beta: ");
  Serial.println(beta);*/
}

void setReports(void) {
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  } else {
    Serial.println("Set accelerometer report... success!");
  }
}
 