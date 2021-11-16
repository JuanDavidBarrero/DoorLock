#ifndef FinderSensor_h
#define FinderSensor_h

#include "Arduino.h"
#include <Adafruit_Fingerprint.h>

#define SensorSerial Serial2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&SensorSerial);

class FingerSensor
{
private:
    /* data */
public:
    FingerSensor();
    void initFingerSentor();
};



#endif