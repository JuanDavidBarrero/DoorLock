#ifndef Finger_Sensor_H
#define Finger_Sensor_H

#include "Arduino.h"
#include <Adafruit_Fingerprint.h>
#define SensorSerial Serial2

class FingerSensor
{
private:
    Adafruit_Fingerprint finger = Adafruit_Fingerprint(&SensorSerial);
public:
    FingerSensor();
    void initFingerSentor();
    int enrollFinger(int);
    int verifyFinger(int);
    bool checkDB();
    int identifingFinger();
    int deleteFingerPrintFromDB(int);
};

#endif

