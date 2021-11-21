# Door lock with the ESP32

## Library necesary 
 * adafruit/Adafruit Fingerprint Sensor Library@^2.0.7
 * seithan/Easy Nextion Library@^1.0.6
 * bblanchon/ArduinoJson@^6.18.5

## DB
 Must be a folder name data that contain data.json file it should be empty

 # Important ! 

these changes need to be made if you are working in the **adafruit / Adafruit@^2.0.7** fingerprint sensor library
 in **Adafruit_Fingerpirnt.h** check for these lines of code

 ```
#define FINGERPRINT_LEDON 0x50
#define FINGERPRINT_LEDOFF 0x51 
```

if these do not exist add them to the beginning of the file, then declare the following functions

```
  uint8_t OpenLED(void);
  uint8_t CloseLED(void);
```


finally add the following lines of code in the file **Adafruit_Fingerpirnt.cpp**

```
uint8_t Adafruit_Fingerprint::OpenLED(void) {
  SEND_CMD_PACKET(FINGERPRINT_LEDON);
}

uint8_t Adafruit_Fingerprint::CloseLED(void) {
  SEND_CMD_PACKET(FINGERPRINT_LEDOFF);
}
```

now you can turn on the sensor led with the ```finger.OpenLed()``` method and turn it off with ```finger.CloseLed()```