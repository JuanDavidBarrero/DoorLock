#ifndef Finger_Sensor_H
#define Finger_Sensor_H

#include "Arduino.h"
#include <Adafruit_Fingerprint.h>
#define SensorSerial Serial2

class FingerSensor
{
private:
    Adafruit_Fingerprint finger = Adafruit_Fingerprint(&SensorSerial);
    int getFingerprintEnroll(int);
public:
    FingerSensor();
    void initFingerSentor();
    void enrollFinger(int);
    bool checkDB();
    int  identifingFinger();
};

#endif