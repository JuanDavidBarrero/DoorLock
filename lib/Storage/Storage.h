#ifndef Storage_H
#define Storage_H

#include <Arduino.h>
#include <SPIFFS.h>

class Storage
{
private:
    /* data */
public:
    Storage(/* args */);
    void initSPIFFS();
    File openData();
    bool saveData(String);
};






#endif