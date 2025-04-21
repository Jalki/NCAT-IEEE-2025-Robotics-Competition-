#include "Arduino_BMI270_BMM150.h"
#include <mbed.h>
#include <rtos.h>

using namespace mbed;
using namespace rtos;

// — Global mutex to serialize all motion functions —
Mutex movemutex;

// — Thread & Prototypes —
Thread headingThread;
void headingTask();
void calibrateBias();
float get_rotation_snapshot();
void rotate_gyro(float targetAngle);
void stop_movement();
void move_forward(float inches);
void move_backward(float inches);
void move_Strafe_Left(float inches);
void move_Strafe_Right(float inches);
void move_Rotate_Clockwise();
void move_Rotate_CounterClockwise();
bool isStationary(float ax, float ay, float az);
void move(float x, float y);

// — Motor pins & globals —
int Motor1For = 2,  Motor1Back = 3;
int Motor2For = 9,  Motor2Back = 8;
int Motor3For = 7,  Motor3Back = 6;
int Motor4For = 4,  Motor4Back = 5;

const float tickrate = 71.42f;
float currentAngle = 0.0f;
float accelX, accelY, accelZ;
float velocityX = 0, velocityY = 0, velocityZ = 0;
float positionX = 0, positionY = 0, positionZ = 0;
unsigned long lastTime = 0;
float dt = 0;
float biasX = 0, biasY = 0, biasZ = 0;
float alpha = 0.1f;
float filteredAccelX = 0, filteredAccelY = 0, filteredAccelZ = 0;
float stationaryThreshold = 0.05f;
float dampingFactor = 0.02f;

void setup() {
    Serial.begin(9600);
    Serial1.begin(9600);

    if (!IMU.begin()) {
        Serial.println("failed to initialize imu!");
        while (1);
    }
    Serial.println("imu initialized!");

    calibrateBias();
    lastTime = millis();

    pinMode(Motor1For, OUTPUT);
    pinMode(Motor1Back, OUTPUT);
    pinMode(Motor2For, OUTPUT);
    pinMode(Motor2Back, OUTPUT);
    pinMode(Motor3For, OUTPUT);
    pinMode(Motor3Back, OUTPUT);
    pinMode(Motor4For, OUTPUT);
    pinMode(Motor4Back, OUTPUT);

    headingThread.start(headingTask);
}

void loop() {
    //Serial.print("Starting Tasks!");
    // Read acceleration data
    if (IMU.accelerationAvailable()) {
        IMU.readAcceleration(accelX, accelY, accelZ);

        // Subtract gravity from Z-axis (if sensor is not level)
        accelZ -= 9.81;

        // Apply low-pass filter
        filteredAccelX = alpha * accelX + (1 - alpha) * filteredAccelX;
        filteredAccelY = alpha * accelY + (1 - alpha) * filteredAccelY;
        filteredAccelZ = alpha * accelZ + (1 - alpha) * filteredAccelZ;

        // Calculate time step (dt) in seconds
        unsigned long currentTime = millis();
        dt = (currentTime - lastTime) / 1000.0; // Convert ms to seconds
        lastTime = currentTime;

        // Check if the sensor is stationary
        if (isStationary(filteredAccelX, filteredAccelY, filteredAccelZ)) {
            // Reset velocity when stationary
            velocityX = 0;
            velocityY = 0;
            velocityZ = 0;
            Serial.println("Sensor is stationary. Resetting velocity.");
        } else {
            // Integrate acceleration to get velocity (v = v0 + a * dt)
            velocityX += filteredAccelX * dt;
            velocityY += filteredAccelY * dt;
            velocityZ += filteredAccelZ * dt;

            // Apply velocity damping
            velocityX *= (1 - dampingFactor);
            velocityY *= (1 - dampingFactor);
            velocityZ *= (1 - dampingFactor);

            // Integrate velocity to get position
            positionX += velocityX * dt;
            positionY += velocityY * dt;
            positionZ += velocityZ * dt;
        }

        // (Optional) Print the results
        // Serial.print("Position X: "); Serial.println(positionX);
    } // end accel check

    if (Serial1.available()) {
        char input[50];
        int bytesRead = Serial1.readBytesUntil('\n', input, sizeof(input) - 1);
        input[bytesRead] = '\0';
        float x, y, rotation;
        int parsed = sscanf(input, "%f,%f,%f", &x, &y, &rotation);

        if (parsed == 3) { // Ensure all three values were received
            Serial.println(String("Received X: ") + x);
            Serial.println(String("Received Y: ") + y);
            Serial.println(String("Received Rotation: ") + rotation);

            if ((x != 0.00f) || (y != 0.00f)) {
                move(x, y);
            } else {
                Serial.print("No x or y to move to!");
                
            }

            if (rotation != 0.00f) {
                rotate_gyro(rotation);
            } else {
                Serial.print("No rotation to rotate to!");
            }

            if (x== 0.00f && y == 0.00f && rotation == 0.00f) {
              Serial.print("Absolutely nothing!");
              Serial1.println("complete");
            }
        } else {
            Serial.println("Data parse error!");
        }
    }
}
bool isStationary(float ax, float ay, float az) {
    float mag = sqrt(ax*ax + ay*ay + az*az);
    return fabs(mag) < stationaryThreshold;
}

void calibrateBias() {
    float sx = 0, sy = 0, sz = 0;
    for (int i = 0; i < 1500; i++) {
        if (IMU.accelerationAvailable()) {
            IMU.readAcceleration(accelX, accelY, accelZ);
            sx += accelX; sy += accelY; sz += accelZ;
        }
        ThisThread::sleep_for(10);
    }
    biasX = sx / 1500.0f;
    biasY = sy / 1500.0f;
    biasZ = sz / 1500.0f;
}

void headingTask() {
    while (true) {
        get_rotation_snapshot();
        ThisThread::sleep_for(10);
    }
}

float get_rotation_snapshot() {
    static unsigned long prev = millis();
    float gx, gy, gz;
    if (IMU.gyroscopeAvailable()) {
        IMU.readGyroscope(gx, gy, gz);
        unsigned long now = millis();
        float d = (now - prev) / 1000.0f;
        prev = now;
        currentAngle += gz * d;
    }
    return currentAngle;
}

void rotate_gyro(float targetAngle) {
    movemutex.lock();

    // 1) Measure drift
    float drift = 0, gx, gy, gz;
    unsigned long prev = millis();
    for (int i = 0; i < 70; i++) {
        if (IMU.gyroscopeAvailable()) {
            IMU.readGyroscope(gx, gy, gz);
            drift += gz;
        }
        delay(10);
    }
    drift /= 70.0f;

    // 2) Start rotation
    if (targetAngle > 0) move_Rotate_Clockwise();
    else                move_Rotate_CounterClockwise();

    // 3) Integrate until target reached
    float angleRotated = 0;
    while (fabs(angleRotated) < fabs(targetAngle)) {
        if (IMU.gyroscopeAvailable()) {
            IMU.readGyroscope(gx, gy, gz);
            gz -= drift;
            unsigned long now = millis();
            float d = (now - prev) / 1000.0f;
            prev = now;
            angleRotated += gz * d;
        }
    }

    stop_movement();

    Serial1.println("complete");
    movemutex.unlock();
}

void move_forward(float inches) {
    movemutex.lock();

    float tickTime  = (inches * tickrate) / 60.0f;
    float desired   = get_rotation_snapshot();
    float deviation = desired;
    stop_movement();

    for (int i = 0; i < 60; i++) {
        // drive forward
        digitalWrite(Motor1For, HIGH);  digitalWrite(Motor1Back, LOW);
        digitalWrite(Motor2For, HIGH);  digitalWrite(Motor2Back, LOW);
        digitalWrite(Motor3For, HIGH);  digitalWrite(Motor3Back, LOW);
        digitalWrite(Motor4For, HIGH);  digitalWrite(Motor4Back, LOW);

        delay((int)round(tickTime));

        // original deviation check
        float now = get_rotation_snapshot();
        deviation = desired - now;
        if (fabs(deviation) > 2.0f) {
            Serial.println("DEviation detected");
        }
    }

    stop_movement();
    movemutex.unlock();
    rotate_gyro(-deviation);
    stop_movement();
}

void move_backward(float inches) {
    movemutex.lock();

    float tickTime  = (inches * tickrate) / 60.0f;
    float desired   = get_rotation_snapshot();
    float deviation = desired;
    stop_movement();

    for (int i = 0; i < 60; i++) {
        // drive backward
        digitalWrite(Motor1For, LOW);   digitalWrite(Motor1Back, HIGH);
        digitalWrite(Motor2For, LOW);   digitalWrite(Motor2Back, HIGH);
        digitalWrite(Motor3For, LOW);   digitalWrite(Motor3Back, HIGH);
        digitalWrite(Motor4For, LOW);   digitalWrite(Motor4Back, HIGH);

        delay((int)round(tickTime));

        float now = get_rotation_snapshot();
        deviation = desired - now;
        if (fabs(deviation) > 2.0f) {
            Serial.println("DEviation detected");
        }
    }

    stop_movement();
    movemutex.unlock();
    rotate_gyro(-deviation);
    stop_movement();

}

void move_Strafe_Left(float inches) {
    movemutex.lock();

    float tickTime  = (inches * tickrate) / 60.0f;
    float desired   = get_rotation_snapshot();
    float deviation = desired;
    stop_movement();

    for (int i = 0; i < 60; i++) {
        // strafe left
        digitalWrite(Motor1For, LOW);  digitalWrite(Motor1Back, HIGH);
        digitalWrite(Motor2For, LOW);  digitalWrite(Motor2Back, HIGH);
        digitalWrite(Motor3For, HIGH); digitalWrite(Motor3Back, LOW);
        digitalWrite(Motor4For, HIGH); digitalWrite(Motor4Back, LOW);

        delay((int)round(tickTime));

        float now = get_rotation_snapshot();
        deviation = desired - now;
        if (fabs(deviation) > 2.0f) {
            Serial.println("DEviation detected");
        }
    }

    stop_movement();

    movemutex.unlock();
}

void move_Strafe_Right(float inches) {
    movemutex.lock();

    float tickTime  = (inches * tickrate) / 60.0f;
    float desired   = get_rotation_snapshot();
    float deviation = desired;
    stop_movement();

    for (int i = 0; i < 60; i++) {
        // strafe right
        digitalWrite(Motor1For, HIGH); digitalWrite(Motor1Back, LOW);
        digitalWrite(Motor2For, HIGH); digitalWrite(Motor2Back, LOW);
        digitalWrite(Motor3For, LOW);  digitalWrite(Motor3Back, HIGH);
        digitalWrite(Motor4For, LOW);  digitalWrite(Motor4Back, HIGH);

        delay((int)round(tickTime));

        float now = get_rotation_snapshot();
        deviation = desired - now;
        if (fabs(deviation) > 2.0f) {
            Serial.println("DEviation detected");
        }
    }

    stop_movement();

    movemutex.unlock();
}

void move_Rotate_Clockwise() {
    digitalWrite(Motor1For, HIGH);  digitalWrite(Motor1Back, LOW);
    digitalWrite(Motor2For, LOW);   digitalWrite(Motor2Back, HIGH);
    digitalWrite(Motor3For, LOW);   digitalWrite(Motor3Back, HIGH);
    digitalWrite(Motor4For, HIGH);  digitalWrite(Motor4Back, LOW);
}

void move_Rotate_CounterClockwise() {
    digitalWrite(Motor1For, LOW);   digitalWrite(Motor1Back, HIGH);
    digitalWrite(Motor2For, HIGH);  digitalWrite(Motor2Back, LOW);
    digitalWrite(Motor3For, HIGH);  digitalWrite(Motor3Back, LOW);
    digitalWrite(Motor4For, LOW);   digitalWrite(Motor4Back, HIGH);
}

void stop_movement() {
    digitalWrite(Motor1For, LOW);  digitalWrite(Motor1Back, LOW);
    digitalWrite(Motor2For, LOW);  digitalWrite(Motor2Back, LOW);
    digitalWrite(Motor3For, LOW);  digitalWrite(Motor3Back, LOW);
    digitalWrite(Motor4For, LOW);  digitalWrite(Motor4Back, LOW);
}

void move(float x, float y) {
    // strafe on X axis
    if (x < 0.0f) {
        Serial.println("I have initiated a movement. This usually means any current movement is stopped.");
        move_Strafe_Left(fabs(x));
    }
    else if (x > 0.0f) {
        Serial.println("I have initiated a movement. This usually means any current movement is stopped2.");
        move_Strafe_Right(x);
    }

    // move on Y axi
    if (y < 0.0f) {
        Serial.println("I have initiated a movement. This usually means any current movement is stopped3.");
        move_forward(fabs(y));
    }
    else if (y > 0.0f) {
        Serial.println("I have initiated a movement. This usually means any current movement is stopped4.");
        move_backward(y);
    }

    Serial.println("Movement complete?");
    Serial1.println("Complete");  // let the sender know movement is done
}

