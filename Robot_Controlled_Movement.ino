#include "Arduino_BMI270_BMM150.h"

#include "Arduino_BMI270_BMM150.h"

// Motor pins
int Motor1For = 2;
int Motor1Back = 3;
int Motor2For = 9;
int Motor2Back = 8;
int Motor3For = 7;
int Motor3Back = 6;
int Motor4For = 4;
int Motor4Back = 5;

// Variables for integration
float accelX, accelY, accelZ; // Raw acceleration values
float velocityX = 0, velocityY = 0, velocityZ = 0; // Integrated velocity
float positionX = 0, positionY = 0, positionZ = 0; // Integrated position
unsigned long lastTime = 0; // Timestamp for the last reading
float dt = 0; // Time step (in seconds)

// Calibration values (measured when stationary)
float biasX = 0, biasY = 0, biasZ = 0;

// Low-pass filter variables
float alpha = 0.1; // Filter coefficient (adjust as needed)
float filteredAccelX = 0, filteredAccelY = 0, filteredAccelZ = 0;

// Stationary detection threshold
float stationaryThreshold = 0.05; // Adjust based on noise level ONLY CHANGE THIS IF YOU HAVE A DIFFERENT SENSOR, YOUR NOISE WILL BE DIFFERENT!

// Velocity damping factor
float dampingFactor = 0.02; // Adjust as needed reduce if velocity is changing to fast, increase if velocity is changing to slow

// Movement control
enum State { FORWARD, BACKWARD, LEFT, RIGHT, STOP };
State currentState = FORWARD;
float targetDistance = 0.01; // Target distance in meters (1 cm)
bool movementComplete = false;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Initialize the IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.println("IMU initialized!");
  calibrateBias(); // Perform bias calibration
  lastTime = millis(); // Initialize the timestamp

  // Set motor pins as outputs
  pinMode(Motor1For, OUTPUT);
  pinMode(Motor1Back, OUTPUT);
  pinMode(Motor2For, OUTPUT);
  pinMode(Motor2Back, OUTPUT);
  pinMode(Motor3For, OUTPUT);
  pinMode(Motor3Back, OUTPUT);
  pinMode(Motor4For, OUTPUT);
  pinMode(Motor4Back, OUTPUT);
}

void loop() {
  // Read acceleration data
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(accelX, accelY, accelZ);

    // Subtract bias
    accelX -= biasX;
    accelY -= biasY;
    accelZ -= biasZ;

    // Subtract gravity from Z-axis (if sensor is not level)
    accelZ -= 9.81;

    // Apply low-pass filter
    filteredAccelX = alpha * accelX + (1 - alpha) * filteredAccelX;
    filteredAccelY = alpha * accelY + (1 - alpha) * filteredAccelY;
    filteredAccelZ = alpha * accelZ + (1 - alpha) * filteredAccelZ;

    // Calculate time step (dt) in seconds
    unsigned long currentTime = millis();
    dt = (currentTime - lastTime) / 1000.0; // Convert milliseconds to seconds
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

      // Integrate velocity to get position (s = s0 + v * dt)
      positionX += velocityX * dt;
      positionY += velocityY * dt;
      positionZ += velocityZ * dt;
    }

    // Print the results
    Serial.print("Position X: ");
    Serial.print(positionX);
    Serial.print(" m, Y: ");
    Serial.print(positionY);
    Serial.print(" m, Z: ");
    Serial.print(positionZ);
    Serial.println(" m");

    // Control movement based on state
    switch (currentState) {
      case FORWARD:
        move_forward();
        if (positionX >= targetDistance) {
          stop_movement();
          currentState = BACKWARD;
          positionX = 0; // Reset position for next movement
        }
        break;

      case BACKWARD:
        move_backward();
        if (positionX <= -targetDistance) {
          stop_movement();
          currentState = LEFT;
          positionX = 0; // Reset position for next movement
        }
        break;

      case LEFT:
        rotate_gyro(180.0);
        if (positionY >= targetDistance) {
          stop_movement();
          currentState = RIGHT;
          positionY = 0; // Reset position for next movement
        }
        break;

      case RIGHT:
        rotate_gyro(-360.0);
        if (positionY <= -targetDistance) {
          stop_movement();
          currentState = FORWARD;
        }
        break;

      case STOP:
        // Do nothing
        break;
    }
  }
}

void rotate_gyro(float targetAngle) {
    float angleRotated = 0;
    float gx, gy, gz;
    float gyroDrift = 0;
    unsigned long prevTime = millis();

    // **1. Measure drift before rotation**  
    int numSamples = 50;
    for (int i = 0; i < numSamples; i++) {
        if (IMU.gyroscopeAvailable()) {
            IMU.readGyroscope(gx, gy, gz);
            gyroDrift += gz;  // Accumulate Z-axis drift
        }
        delay(10);  // Small delay between samples
    }
    gyroDrift /= numSamples;  // Average drift over samples

    // **2. Determine rotation direction**
    bool clockwise = (targetAngle > 0);
    if (clockwise) {
        move_Rotate_Clockwise();
    } else {
        move_Rotate_CounterClockwise();
    }

    // **3. Rotate while compensating for drift**
    while (abs(angleRotated) <= abs(targetAngle)) {  
        if (IMU.gyroscopeAvailable()) {
            IMU.readGyroscope(gx, gy, gz);
            

            // Apply drift correction
            gz -= gyroDrift;

            // Compute time elapsed
            unsigned long currentTime = millis();
            float deltaTime = (currentTime - prevTime) / 1000.00; // Convert ms to seconds
            prevTime = currentTime;

            // Integrate gyroscope data to estimate angle rotated
            angleRotated += gz * deltaTime;
            Serial.println(angleRotated);
            //positionalData[2] = angleRotated;
        }
    }
    stop_movement();  // Stop once the target angle is reached
}

// Function to check if the sensor is stationary
bool isStationary(float accelX, float accelY, float accelZ) {
  // Compute the magnitude of acceleration
  float accelMagnitude = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

  // If the magnitude is close to 0 (within the threshold), the sensor is stationary
  return (abs(accelMagnitude) < stationaryThreshold);
}

// Function to calibrate bias
void calibrateBias() {
  int numSamples = 1000; // Increase the number of samples
  float sumX = 0, sumY = 0, sumZ = 0;

  for (int i = 0; i < numSamples; i++) {
    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(accelX, accelY, accelZ);
      sumX += accelX;
      sumY += accelY;
      sumZ += accelZ;
    }
    delay(10); // Wait between samples
  }

  biasX = sumX / numSamples;
  biasY = sumY / numSamples;
  biasZ = sumZ / numSamples;

  Serial.print("Calibrated Bias X: ");
  Serial.print(biasX);
  Serial.print(", Y: ");
  Serial.print(biasY);
  Serial.print(", Z: ");
  Serial.println(biasZ);
}

// Motor control functions (unchanged)
void move_forward() {
  digitalWrite(Motor1For, HIGH);
  digitalWrite(Motor1Back, LOW);
  digitalWrite(Motor2For, HIGH);
  digitalWrite(Motor2Back, LOW);
  digitalWrite(Motor3For, HIGH);
  digitalWrite(Motor3Back, LOW);
  digitalWrite(Motor4For, HIGH);
  digitalWrite(Motor4Back, LOW);
}

void move_backward() {
  digitalWrite(Motor1For, LOW);
  digitalWrite(Motor1Back, HIGH);
  digitalWrite(Motor2For, LOW);
  digitalWrite(Motor2Back, HIGH);
  digitalWrite(Motor3For, LOW);
  digitalWrite(Motor3Back, HIGH);
  digitalWrite(Motor4For, LOW);
  digitalWrite(Motor4Back, HIGH);
}

void move_Strafe_Left() {
  digitalWrite(Motor1For, LOW);
  digitalWrite(Motor1Back, HIGH);
  digitalWrite(Motor2For, LOW);
  digitalWrite(Motor2Back, HIGH);
  digitalWrite(Motor3For, HIGH);
  digitalWrite(Motor3Back, LOW);
  digitalWrite(Motor4For, HIGH);
  digitalWrite(Motor4Back, LOW);
}

void move_Strafe_Right() {
  digitalWrite(Motor1For, HIGH);
  digitalWrite(Motor1Back, LOW);
  digitalWrite(Motor2For, HIGH);
  digitalWrite(Motor2Back, LOW);
  digitalWrite(Motor3For, LOW);
  digitalWrite(Motor3Back, HIGH);
  digitalWrite(Motor4For, LOW);
  digitalWrite(Motor4Back, HIGH);
}

//Moves Clockwise
void move_Rotate_Clockwise(){
  digitalWrite(Motor1For, HIGH);
  digitalWrite(Motor1Back, LOW);
  //analogWrite(D1ENA1pin, pwm * 0.50);

  digitalWrite(Motor2For, LOW);
  digitalWrite(Motor2Back, HIGH);
  //analogWrite(D1ENA2pin, pwm * 0.50);

  digitalWrite(Motor3For, LOW);
  digitalWrite(Motor3Back, HIGH);
  //analogWrite(D2ENA1pin, pwm * 0.50);

  digitalWrite(Motor4For, HIGH);
  digitalWrite(Motor4Back, LOW);
  //analogWrite(D2ENA2pin, pwm * 0.50);
}
//Moves Counter Clockwise
void move_Rotate_CounterClockwise(){  
  digitalWrite(Motor1For, LOW);
  digitalWrite(Motor1Back, HIGH);
  

  digitalWrite(Motor2For, HIGH);
  digitalWrite(Motor2Back, LOW);
  

  digitalWrite(Motor3For, HIGH);
  digitalWrite(Motor3Back, LOW);
  

  digitalWrite(Motor4For, LOW);
  digitalWrite(Motor4Back, HIGH);

}

void stop_movement() {
  digitalWrite(Motor1For, LOW);
  digitalWrite(Motor1Back, LOW);
  digitalWrite(Motor2For, LOW);
  digitalWrite(Motor2Back, LOW);
  digitalWrite(Motor3For, LOW);
  digitalWrite(Motor3Back, LOW);
  digitalWrite(Motor4For, LOW);
  digitalWrite(Motor4Back, LOW);
}