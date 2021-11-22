#include <Arduino.h>
#include <EasyNextionLibrary.h>
#include <ArduinoJson.h>
#include <MFRC522.h>

#include <Finger_Sensor.h>
#include <Storage.h>

#define RST_PIN 9
#define SS_PIN 5
MFRC522 mfrc522(SS_PIN, RST_PIN);

Storage DB;
DynamicJsonDocument familyData(450);
DynamicJsonDocument newMember(200);

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
void onTimer();

#define NextionScreen Serial1
EasyNex myNex(NextionScreen);

FingerSensor Sensor;
int personID = -1;
bool timeOut = false;

struct Family
{
  String name;
  int id;
  byte cardUID[4];
};
Family members[127];

void prossesAndSaveData(String, int, byte *);
void loadInfo(File);
int seekMatch(int);
void deleteAndSaveData(int);
bool validateAction();
bool checkUIDRFID(byte *);
byte *UIDfromCard();

void setup()
{
  Serial.begin(115200);
  myNex.begin(9600);
  DB.initSPIFFS();
  SPI.begin();
  mfrc522.PCD_Init();

  File data = DB.openData();
  loadInfo(data);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);

  Sensor.initFingerSentor();
  if (!Sensor.checkDB())
  {
    Sensor.enrollFinger(1);
  }
}

void loop()
{
  myNex.NextionListen();
}

void trigger0()
{
  timerWrite(timer, 0);
  timerAlarmWrite(timer, 10000000, false);
  timerAlarmEnable(timer);
  myNex.writeStr("page 5");

  while (personID == -1)
  {
    personID = Sensor.identifingFinger();
    if (timeOut)
    {
      myNex.writeStr("page 9");
      delay(1000);
      myNex.writeStr("page 0");
      timeOut = false;
      timerWrite(timer, 0);
      return;
    }
  }

  timerAlarmDisable(timer);
  timeOut = false;

  int id = seekMatch(personID);

  myNex.writeStr("page 1");
  myNex.writeStr("t1.txt", members[id].name);
  personID = -1;
  delay(3000);
  myNex.writeStr("page 0");
}

void trigger1()
{
  if (!validateAction())
    return;
  myNex.writeStr("page 12");
  delay(1000);
  myNex.writeStr("page 2");
  String nombre = myNex.readStr("t1.txt");
  int id = myNex.readNumber("n0.val");
  bool card = myNex.readNumber("n1.val");

  if (id == 0 || id > 127)
    return;
  myNex.writeStr("page 5");
  Sensor.enrollFinger(id);
  myNex.writeStr("page 6");
  Sensor.verifyFinger(id);
  myNex.writeStr("page 7");

  if (card)
  {
    byte *password = UIDfromCard();
    prossesAndSaveData(nombre, id, password);
  }
  else{
    byte *noPassword = {0x0};
    prossesAndSaveData(nombre, id, noPassword);
  }

  

  delay(1500);
  myNex.writeStr("page 2");
}

void trigger2()
{
  char t = 't';
  for (int i = 0; i < 6; i++)
  {
    String data = t + String(i + 1) + ".txt";
    if (members[i].name == "")
      continue;
    myNex.writeStr(data, members[i].name);
  }
}

void trigger3()
{
  char t = 't';
  for (int i = 0; i < 6; i++)
  {
    String data = t + String(i + 1) + ".pco";
    int value = myNex.readNumber(data);
    if (value == 63488)
    {
      if (!validateAction())
        return;

      Sensor.deleteFingerPrintFromDB(members[i].id);
      deleteAndSaveData(i);
      myNex.writeStr("page 8");
      delay(1000);
      myNex.writeStr("page 0");
      return;
    }
  }
}

bool validateAction()
{

  File data = DB.openData();
  if (!data)
    return true;

  deserializeJson(familyData, data);

  if (data.size() == 0)
    return true;

  Serial.println("aca presente");
  timerWrite(timer, 0);
  timerAlarmWrite(timer, 10000000, false);
  timerAlarmEnable(timer);
  myNex.writeStr("page 11");
  while (personID == -1)
  {
    personID = Sensor.identifingFinger();
    if (timeOut)
    {
      myNex.writeStr("page 9");
      delay(1000);
      myNex.writeStr("page 0");
      timeOut = false;
      timerWrite(timer, 0);
      return false;
    }
  }
  personID = -1;
  timerAlarmDisable(timer);
  DB.closeData(data);
  return true;
}

void IRAM_ATTR onTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);
  timeOut = true;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void prossesAndSaveData(String nombre, int id, byte *password)
{
  String info = "";
  newMember["name"] = nombre;
  newMember["id"] = id;
  for (int i = 0; i < 4; i++)
  {
    newMember["cardUID"][i] = password[i];
  }
  
  familyData.add(newMember);
  serializeJson(familyData, info);
  if (DB.saveData(info))
  {
    Serial.println("Información guardada");
    File data = DB.openData();
    loadInfo(data);
  }
}

void loadInfo(File data)
{
  deserializeJson(familyData, data);
  int size = familyData.size();
  if (size == 0)
    return;
  for (int i = 0; i < size; i++)
  {
    String nombre = familyData[i]["name"];
    members[i].name = nombre;
    members[i].id = familyData[i]["id"];
    for (int j = 0; j < 4; j++)
    {
      members[i].cardUID[j] = familyData["cardUID"][j];
    }
  }
  DB.closeData(data);
}

int seekMatch(int personID)
{
  int size = sizeof(members) / sizeof(members[0]);

  for (int i = 0; i < size; i++)
  {
    if (members[i].id == personID)
    {
      return i;
    }
  }
  return -1;
}

void deleteAndSaveData(int id)
{
  String info = "";
  familyData.remove(id);
  serializeJson(familyData, info);
  if (DB.saveData(info))
  {
    Serial.println("Información guardada");
    File data = DB.openData();
    members[id].name = "";
    members[id].id = 0;
    loadInfo(data);
  }
}

bool checkUIDRFID(byte *password)
{
  for (int i = 0; i < 4; i++)
  {
    if (password[i] != mfrc522.uid.uidByte[i])
    {
      return false;
    }
  }
  return true;
}

byte *UIDfromCard()
{
  byte *arr = new byte[4];
  for (int i = 0; i < 4; i++)
  {
    arr[i] = mfrc522.uid.uidByte[i];
  }
  return arr;
}
