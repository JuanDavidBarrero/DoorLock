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
  int cardUID[4];
};
Family members[127];

void prossesAndSaveData(String, int, int *);
void loadInfo(File);
int seekMatch(int);
void deleteAndSaveData(int);
bool validateAction();
bool confirmUID(int *);
int *UIDfromCard();
bool checkCardAvailable();
int seekUserbyUID(int *);

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

/* Verificar usuario este en DB */
void trigger0()
{
  timerWrite(timer, 0);
  timerAlarmWrite(timer, 10000000, false);
  timerAlarmEnable(timer);
  myNex.writeStr("page 16");

  while (personID == -1)
  {
    personID = Sensor.identifingFinger();

    if (checkCardAvailable())
    {
      int *uid = UIDfromCard();
      if (confirmUID(uid))
      {
        int id = seekUserbyUID(uid);
        myNex.writeStr("page 1");
        myNex.writeStr("t1.txt", members[id].name);
        timerAlarmDisable(timer);
        Sensor.controlLed();
        delay(3000);
        myNex.writeStr("page 0");
        return;
      }
    }

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

/* Añadir usuario a DB */
void trigger1()
{
  if (!validateAction())
    return;
  myNex.writeStr("page 12");
  delay(1000);
  myNex.writeStr("page 2");
  String nombre = myNex.readStr("t1.txt");
  int id = myNex.readNumber("n0.val");
  int isCard = myNex.readNumber("r0.val");

  if (id == 0 || id > 127)
    return;
  myNex.writeStr("page 5");
  Sensor.enrollFinger(id);
  myNex.writeStr("page 6");
  Sensor.verifyFinger(id);

  if (isCard)
  {
    myNex.writeStr("page 15");
    while (true)
    {
      if (checkCardAvailable())
      {
        int *password = UIDfromCard();
        prossesAndSaveData(nombre, id, password);
        myNex.writeStr("page 7");
        break;
      }
    }
  }
  else
  {
    int *noPassword = NULL;
    prossesAndSaveData(nombre, id, noPassword);
  }

  delay(2000);
  myNex.writeStr("page 0");
}

/* Cargar usurios en pantalla */
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

/* Eliminar usuario de DB */
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

  timerWrite(timer, 0);
  timerAlarmWrite(timer, 10000000, false);
  timerAlarmEnable(timer);
  myNex.writeStr("page 11");
  while (personID == -1)
  {
    personID = Sensor.identifingFinger();
    if (checkCardAvailable())
    {
      int *uid = UIDfromCard();
      if (confirmUID(uid))
      {
        timerAlarmDisable(timer);
        Sensor.controlLed();
        return true;
      }
    }
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

void prossesAndSaveData(String nombre, int id, int *password)
{
  String info = "";
  newMember["name"] = nombre;
  newMember["id"] = id;

  if (password == NULL)
  {
    for (int i = 0; i < 4; i++)
    {
      newMember["cardUID"][i] = 0;
    }
  }
  else
  {
    for (int i = 0; i < 4; i++)
    {
      newMember["cardUID"][i] = password[i];
    }
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
      int uid = familyData[i]["cardUID"][j];
      members[i].cardUID[j] = uid;
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
    for (int i = 0; i < 4; i++)
    {
      members[id].cardUID[i] = 0;
    }

    loadInfo(data);
  }
}

bool confirmUID(int *password)
{
  int count = 0;
  for (int i = 0; i < 127; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      if (password[j] == members[i].cardUID[j])
      {
        count++;
      }
    }
    if (count == 4)
    {
      return true;
    }
    count = 0;
  }

  return false;
}

int *UIDfromCard()
{
  int *arr = new int[4];
  for (int i = 0; i < 4; i++)
  {
    arr[i] = mfrc522.uid.uidByte[i];
  }
  return arr;
}

int seekUserbyUID(int *uid)
{
  int size = sizeof(members) / sizeof(members[0]);

  int count = 0;
  for (int i = 0; i < size; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      if (uid[j] == members[i].cardUID[j])
      {
        count++;
      }
    }
    if (count == 4)
    {
      return i;
    }
    count = 0;
  }

  return -1;
}

bool checkCardAvailable()
{
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return false;
  }

  if (!mfrc522.PICC_ReadCardSerial())
  {
    return false;
  }

  return true;
}