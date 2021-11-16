#include <Arduino.h>
#include <Finger_Sensor.h>

FingerSensor Sensor;

void setup() {
  Serial.begin(115200);
  Sensor.initFingerSentor();
}

void loop() {
  // put your main code here, to run repeatedly:
}