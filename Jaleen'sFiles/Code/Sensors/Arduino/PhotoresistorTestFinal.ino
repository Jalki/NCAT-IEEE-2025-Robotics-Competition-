const int photoPin = A0; // Analog pin connected to the photoresistor

void setup() {
  Serial.begin(9600); // Initialize serial communication at 9600 baud
  
  // Set analog pin A0 as input with internal pull-up resistor
  pinMode(photoPin, INPUT_PULLUP);
}

void loop() {
  int sensorValue = analogRead(photoPin); // Read the analog value from the photoresistor
  float voltage = sensorValue * (5.0 / 1023.0); // Convert the analog value to voltage
  float resistance = voltage * 50000 / (5.0 - voltage); //(5.0 - voltage) * 50000 / voltage; // Calculate the resistance of the photoresistor

  Serial.print("Voltage: ");
  Serial.print(voltage);
  Serial.print(" V, Resistance: ");
  Serial.print(resistance);
  Serial.println(" ohms");

  delay(500); // Wait for 500 milliseconds before the next reading
}

