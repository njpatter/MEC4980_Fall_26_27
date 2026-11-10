# NAU7802 load cell reader

Connect the NAU7802 breakout to the Feather ESP32-S3's 3.3 V, GND, SDA,
and SCL pins, and connect the load cell to the breakout according to its
pin labels. The project uses the board's default I2C pins.

## Calibration

1. Leave `isCalibrated` set to `false` in `src/main.cpp`, build and upload,
   then open the serial monitor at 115200 baud. Configure the monitor to send
   a newline after each entry.
2. At each prompt, place the indicated known load on the cell, let it settle,
   and enter its force in newtons. The two forces must be different. An
   unloaded cell can be used as the 0 N point. If the known loads are masses,
   convert them to force first (newtons = kilograms × 9.80665).
3. The program discards one reading and averages 10 fresh readings per point,
   fits a straight line, and saves the coefficients in ESP32 nonvolatile storage.
4. Set `isCalibrated` to `true` and upload again to use the saved coefficients
   on subsequent startups. Leaving it `false` runs calibration on every
   startup. Set it back to `false` to recalibrate.

The program reports the calculated force in newtons every 0.5 seconds when a
conversion is available. If initialization or calibration fails, check the
serial messages and wiring and restart. If a valid saved fit is unavailable,
the program reports an error instead of displaying uncalibrated forces.
