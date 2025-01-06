int linetracking = 10; // Declaration of the sensor input pin
int value; // Temporary variable
   
void setup () {
  pinMode(linetracking, INPUT); // Initialization sensor pin
  digitalWrite(linetracking, HIGH); // Activation of internal pull-up resistor
  Serial.begin(9600); // Initialization of the serial monitor
  Serial.println("KY-033 Linetracking");
}
   
void loop () {
  // The current signal at the sensor is read out.
  value = digitalRead(linetracking);
  // If a signal could be detected, this is displayed on the serial monitor.
  if (value == LOW) {
    Serial.println("Line recognized");
  delay(200); // 200 ms break
  }
}