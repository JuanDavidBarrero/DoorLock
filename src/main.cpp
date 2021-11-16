#include <Arduino.h>
#include <Finger_Sensor.h>

FingerSensor Sensor;
int personID;

void setup()
{
  Serial.begin(115200);
  Sensor.initFingerSentor();
  if (!Sensor.checkDB())
  {
    Sensor.enrollFinger(1);
  }
}

void loop()
{
  personID = Sensor.identifingFinger();
  if(personID != -1){
    Serial.printf("The finger id correspont to %i \n", personID);
    delay(3000);
  }
  Serial.print("Plese place your finger\n");
}