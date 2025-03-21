#include <Arduino_BMI270_BMM150.h>

extern "C" {
  #include "kvstore_global_api.h"
}

const int CALIBRATION_BUTTON_PIN = 7;
bool isMovingStraight = true;  // Replace this with actual logic

float yaw = 0.0;
unsigned long lastTime = 0;

struct MagCalibration {
  float xMin;
  float xMax;
  float yMin;
  float yMax;
};

MagCalibration magCal;
const char* CAL_KEY = "/kv/magCal";

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(CALIBRATION_BUTTON_PIN, INPUT_PULLDOWN);

  if (!IMU.begin()) {
    Serial.println("❌ IMU init failed");
    while (true);
  }

  if (digitalRead(CALIBRATION_BUTTON_PIN) == HIGH) {
    Serial.println("⚙️ Calibration mode — move in a figure 8...");
    calibrateMagnetometer(5000);
    saveCalibration();
  } else {
    if (!loadCalibration()) {
      Serial.println("⚠️ No saved calibration found. Run calibration mode.");
    }
  }

  lastTime = millis();
}

void loop() {
  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0f;
  lastTime = now;

  float gx, gy, gz;
  if (IMU.gyroscopeAvailable() && IMU.readGyroscope(gx, gy, gz)) {
    yaw += gz * 180.0 / PI * dt;
    yaw = wrapAngle(yaw);
  }

  float mx, my, mz;
  if (isMovingStraight && IMU.magneticFieldAvailable() && IMU.readMagneticField(mx, my, mz)) {
    // Apply calibration offset
    mx -= (magCal.xMin + magCal.xMax) / 2;
    my -= (magCal.yMin + magCal.yMax) / 2;

    float magYaw = atan2(my, mx) * 180.0 / PI;
    if (magYaw < 0) magYaw += 360;

    float error = wrapError(magYaw - yaw);
    yaw += 0.05 * error;
    yaw = wrapAngle(yaw);
  }

  Serial.print("Yaw: ");
  Serial.println(yaw);

  delay(50);
}

// ==== Calibration Functions ====

void calibrateMagnetometer(unsigned long duration_ms) {
  float x, y, z;
  unsigned long start = millis();
  magCal.xMin = 1000; magCal.xMax = -1000;
  magCal.yMin = 1000; magCal.yMax = -1000;

  while (millis() - start < duration_ms) {
    if (IMU.magneticFieldAvailable() && IMU.readMagneticField(x, y, z)) {
      magCal.xMin = min(magCal.xMin, x);
      magCal.xMax = max(magCal.xMax, x);
      magCal.yMin = min(magCal.yMin, y);
      magCal.yMax = max(magCal.yMax, y);
    }
  }

  Serial.println("✅ Calibration complete.");
  Serial.print("X offset: "); Serial.println((magCal.xMin + magCal.xMax) / 2);
  Serial.print("Y offset: "); Serial.println((magCal.yMin + magCal.yMax) / 2);
}

void saveCalibration() {
  int result = kv_set(CAL_KEY, &magCal, sizeof(magCal), 0);
  if (result == MBED_SUCCESS) {
    Serial.println("💾 Calibration saved to flash.");
  } else {
    Serial.print("❌ Save failed with error: ");
    Serial.println(result);
  }
}

bool loadCalibration() {
  size_t actual_size = 0;
  int result = kv_get(CAL_KEY, &magCal, sizeof(magCal), &actual_size);
  if (result == MBED_SUCCESS && actual_size == sizeof(magCal)) {
    Serial.println("📂 Calibration loaded from flash.");
    Serial.print("X offset: "); Serial.println((magCal.xMin + magCal.xMax) / 2);
    Serial.print("Y offset: "); Serial.println((magCal.yMin + magCal.yMax) / 2);
    return true;
  } else {
    Serial.print("❌ Load failed with error: ");
    Serial.println(result);
    return false;
  }
}

// ==== Math Helpers ====

float wrapAngle(float angle) {
  while (angle < 0) angle += 360;
  while (angle >= 360) angle -= 360;
  return angle;
}

float wrapError(float error) {
  if (error > 180) error -= 360;
  if (error < -180) error += 360;
  return error;
}
