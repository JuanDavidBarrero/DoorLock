#include "Storage.h"

Storage::Storage(/* args */)
{
}

void Storage::initSPIFFS()
{
    if (!SPIFFS.begin())
    {
        Serial.println("An error has occurred while mounthing spiffs");
    }
    Serial.println("SPIFFS mounted successfully");
}

File Storage::openData(){
    File data = SPIFFS.open("/data.json",FILE_READ);
    return data;
}

void Storage::closeData(File data){
    data.close();
}

bool Storage::saveData(String info){
    if(info.length() < 0) return 0;
    File data = SPIFFS.open("/data.json",FILE_WRITE);
    bool resp = data.println(info);
    data.close();
    return resp;
}