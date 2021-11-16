#include "FingerSensor.h"

FingerSensor::FingerSensor(/* args */)
{
}

void FingerSensor::initFingerSentor()
{
    finger.begin(57600);
    if (!finger.verifyPassword())
    {
        Serial.print("Did NOT find finger print sensor \n");
        while (true)
        {
            Serial.print("Please restart the ESP32\n");
            delay(1000);
        }
        
    }
    Serial.print("Found finger print sensor \n");
}