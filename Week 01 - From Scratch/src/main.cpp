#include <Arduino.h>
#include <time.h>
#include <Adafruit_ST7789.h>
//long getTime();
Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(240, 135);
long startTime = 0;

void setup() {
    display.init(135, 240);
    display.setRotation(3);
    canvas.setTextColor(ST77XX_GREEN);
    pinMode(TFT_BACKLITE, OUTPUT);
    digitalWrite(TFT_BACKLITE, 1);
    //startTime = millis();
}

void loop() {
    canvas.fillScreen(ST77XX_ORANGE);
    canvas.setCursor(0, 20);
    canvas.print("Hello world at time ");
    //long currentTime = getTime();
    //canvas.print(currentTime);
    display.drawRGBBitmap(0,0, canvas.getBuffer(), 240, 135);
    delay(50);
}

long getTime() {
    return (millis() - startTime);
}