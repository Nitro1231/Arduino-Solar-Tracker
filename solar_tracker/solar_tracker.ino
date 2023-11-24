// Including necessary libraries
#include <Servo.h>                // Library for controlling servo motors
#include <ArduinoJson.h>          // Library for handling JSON, used for debugging, https://arduinojson.org/?utm_source=meta&utm_medium=library.properties


// Defining pins for various components
#define PIN_SERVO         3       // ~3,   PWM,      SG90,     Pin for servo motor
#define PIN_LIGHT_LEFT    A0      // A0,   Analog,   10Kohm,   Pin for left LDR
#define PIN_LIGHT_RIGHT   A1      // A1,   Analog,   10Kohm,   Pin for right LDR
#define PIN_TOGGGLE       A2      // A2,   Digital,  switch,   Pin for toggle switch


// Creating a Servo object to control a servo motor
Servo servo;
int angle = 90;                   // Starting angle of the servo
const int threshold = 5;          // Allowed difference between two LDR readings
const int speed = 10;             // Servo Speed
DynamicJsonDocument doc(1024);    // JSON object for serial communication and debugging


// Class to scale sensor readings
class Scaler {
  private:
    int min = 1023;               // Initial value for minimum sensor reading
    int max = 0;                  // Initial value for maximum sensor reading

  public:
    Scaler() {}

    // Function to update min and max based on new value
    void updateMinMax(int value) {
      if (value < min) min = value;
      if (value > max) max = value;
    }

    // Function to scale the sensor value between 0 and 100
    int scaleValue(int value) {
      updateMinMax(value);
      if (max == min) return 0; // Avoid division by zero
      return (int) ((value - min) / (float)(max - min) * 100);
    }
};


Scaler scaler1; // Object for scaling left LDR readings
Scaler scaler2; // Object for scaling right LDR readings


// Arduino setup function, runs once at startup
void setup() {
  Serial.begin(9600);                 // Setup serial communication
  servo.attach(PIN_SERVO);            // Attach servo to its pin
  pinMode(PIN_TOGGGLE, INPUT);        // Set toggle pin as input
  servo.write(servoAngle(angle));     // Move servo to initial angle
}


// Arduino main loop function, runs repeatedly
void loop() {
  int lightLeft = scaler1.scaleValue(analogRead(PIN_LIGHT_LEFT));     // Read and scale left LDR value
  int lightRight = scaler2.scaleValue(analogRead(PIN_LIGHT_RIGHT));   // Read and scale right LDR value
  int toggle = digitalRead(PIN_TOGGGLE); // Read toggle switch state

  // Storing sensor values in JSON document for debugging
  doc["left_ldr"] = lightLeft;
  doc["right_ldr"] = lightRight;
  doc["toggle"] = toggle;

  // Control logic based on LDR readings and toggle switch
  if (toggle == 0) {
    if (abs(lightLeft - lightRight) < threshold) {
      doc["status"] = "fixed";
    } else if (lightLeft > lightRight) {
      doc["status"] = "left";
      angle += speed;
      servo.write(servoAngle(angle));
    } else {
      doc["status"] = "right";
      angle -= speed;
      servo.write(servoAngle(angle));
    }
  } else {
    doc["status"] = "toggled";
  }
  doc["angle"] = angle;
  serializeJsonPretty(doc, Serial); // Output JSON to serial
  Serial.println();
  delay(1000); // Wait for a second
}


// Function to constrain and return servo angle
int servoAngle(int &angle) {
  if (angle < 10) {
    angle = 10; // Minimum angle limit
  } else if (angle >170) {
    angle = 170; // Maximum angle limit
  }
  return angle;
}
