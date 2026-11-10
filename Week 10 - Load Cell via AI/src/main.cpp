#include <Arduino.h>
#include <Adafruit_NAU7802.h>
#include <Preferences.h>
#include <Wire.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>

// Set to true after completing calibration to use the saved coefficients.
const bool isCalibrated = false;
const unsigned long reportIntervalMs = 500;
const unsigned long sampleTimeoutMs = 3000;
const unsigned int calibrationSamples = 10;

struct Calibration {
  double slope;
  double intercept;
};

Adafruit_NAU7802 nau;
Calibration calibration;
bool sensorReady = false;
bool calibrationReady = false;
unsigned long lastReport = 0;

bool validCalibration(const Calibration &value) {
  return isfinite(value.slope) && value.slope != 0 &&
         isfinite(value.intercept);
}

bool saveCalibration(const Calibration &value) {
  Preferences preferences;
  if (!preferences.begin("load-cell", false)) {
    return false;
  }
  bool saved = preferences.putBytes("fit", &value, sizeof(value)) == sizeof(value);
  preferences.end();
  return saved;
}

bool loadCalibration(Calibration &value) {
  Preferences preferences;
  if (!preferences.begin("load-cell", true)) {
    return false;
  }
  bool loaded = preferences.getBytesLength("fit") == sizeof(value) &&
                preferences.getBytes("fit", &value, sizeof(value)) == sizeof(value);
  preferences.end();
  return loaded && validCalibration(value);
}

double readKnownForce(unsigned int point) {
  char line[48];
  size_t length = 0;
  bool tooLong = false;

  Serial.printf("Place load %u, let it settle, then enter its force in N: ",
                point);
  while (true) {
    if (!Serial.available()) {
      delay(1);
      continue;
    }

    char ch = static_cast<char>(Serial.read());
    if (ch == '\r') {
      continue;
    }
    if (ch != '\n') {
      if (length < sizeof(line) - 1) {
        line[length++] = ch;
      } else {
        tooLong = true;
      }
      continue;
    }

    line[length] = '\0';
    errno = 0;
    char *end;
    double force = strtod(line, &end);
    while (isspace(static_cast<unsigned char>(*end))) {
      ++end;
    }
    if (!tooLong && end != line && *end == '\0' && errno != ERANGE &&
        isfinite(force) && force >= 0) {
      return force;
    }
    Serial.println("Enter a finite, non-negative force in newtons.");
    length = 0;
    tooLong = false;
  }
}

bool readSample(int32_t &sample) {
  unsigned long start = millis();
  while (!nau.available()) {
    if (millis() - start >= sampleTimeoutMs) {
      Serial.println("Timed out waiting for an NAU7802 reading.");
      return false;
    }
    delay(1);
  }
  sample = nau.read();
  return true;
}

bool readCalibrationAverage(double &average) {
  int32_t sample;
  if (!readSample(sample)) {
    return false;
  }

  double total = 0;
  for (unsigned int i = 0; i < calibrationSamples; ++i) {
    if (!readSample(sample)) {
      return false;
    }
    total += sample;
  }
  average = total / calibrationSamples;
  return true;
}

bool runCalibration(Calibration &result) {
  while (!Serial) {
    delay(10);
  }
  Serial.println("Two-point calibration: send each force with a newline.");

  while (true) {
    double force1 = readKnownForce(1);
    Serial.println("\nCollecting readings for load 1...");
    double raw1;
    if (!readCalibrationAverage(raw1)) {
      return false;
    }
    Serial.printf("Average raw count: %.2f\n", raw1);

    double force2;
    do {
      force2 = readKnownForce(2);
      if (force2 == force1) {
        Serial.println("\nThe two forces must be different.");
      }
    } while (force2 == force1);

    Serial.println("\nCollecting readings for load 2...");
    double raw2;
    if (!readCalibrationAverage(raw2)) {
      return false;
    }
    Serial.printf("Average raw count: %.2f\n", raw2);

    if (raw1 == raw2) {
      Serial.println("The raw readings are identical. Repeat calibration.");
      continue;
    }

    result.slope = (force2 - force1) / (raw2 - raw1);
    result.intercept = force1 - result.slope * raw1;
    if (!validCalibration(result)) {
      Serial.println("Invalid calibration fit. Repeat calibration.");
      continue;
    }
    if (!saveCalibration(result)) {
      Serial.println("Failed to save calibration to the ESP32.");
      return false;
    }
    Serial.printf("Calibration saved: force (N) = %.9g * raw + %.9g\n",
                  result.slope, result.intercept);
    return true;
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  sensorReady = nau.begin();
  if (!sensorReady) {
    Serial.println("Failed to initialize NAU7802. Check power and I2C wiring.");
  } else if (isCalibrated) {
    calibrationReady = loadCalibration(calibration);
    if (!calibrationReady) {
      Serial.println("No valid saved calibration. Set isCalibrated to false and restart.");
    }
  } else {
    calibrationReady = runCalibration(calibration);
    if (!calibrationReady) {
      Serial.println("Calibration failed. Check the sensor and restart.");
    }
  }
  lastReport = millis();
}

void loop() {
  unsigned long now = millis();
  if (now - lastReport < reportIntervalMs) {
    return;
  }
  lastReport = now;

  if (!sensorReady) {
    Serial.println("NAU7802 not initialized. Check wiring and restart.");
  } else if (!calibrationReady) {
    Serial.println("No valid calibration. Check previous errors and restart.");
  } else if (!nau.available()) {
    Serial.println("NAU7802 reading not ready.");
  } else {
    double force = calibration.slope * nau.read() + calibration.intercept;
    if (isfinite(force)) {
      Serial.printf("Force: %.3f N\n", force);
    } else {
      Serial.println("Calculated force is not finite. Recalibrate the sensor.");
    }
  }
}
