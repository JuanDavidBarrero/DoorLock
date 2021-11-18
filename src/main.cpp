#include <Arduino.h>
#include <EasyNextionLibrary.h>
#include <Finger_Sensor.h>
#include <Storage.h>

Storage DB;

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
void onTimer();

#define NextionScreen Serial1
EasyNex myNex(NextionScreen);

FingerSensor Sensor;
int personID = -1;
bool timeOut = false;
String family[127];

struct Family
{
  String name;
  int id;
};
Family members[6];
int pos = 0;

void setup()
{
  Serial.begin(115200);
  myNex.begin(9600);
  DB.initSPIFFS();

  File data = DB.openData();

  

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
  myNex.writeStr("page 1");
  myNex.writeStr("t1.txt", family[personID - 1]);
  personID = -1;
  delay(3000);
  myNex.writeStr("page 0");
}

void trigger1()
{
  members[pos].name = myNex.readStr("t1.txt");
  members[pos].id = myNex.readNumber("n0.val");
  if (members[pos].id == 0 || members[pos].id > 127)
    return;
  myNex.writeStr("page 5");
  Sensor.enrollFinger(members[pos].id);
  myNex.writeStr("page 6");
  delay(1000);
  myNex.writeStr("page 5");
  Sensor.verifyFinger(members[pos].id);
  myNex.writeStr("page 7");
  delay(1500);
  myNex.writeStr("page 2");
  pos ++;
}

void IRAM_ATTR onTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);
  timeOut = true;
  portEXIT_CRITICAL_ISR(&timerMux);
}
// void trigger2()
// {
//   Serial.printf("id para ser eliminado\n");
//   int id = myNex.readNumber("t1.pco");
//   Serial.printf("El nombre enviado fue %i\n", id);
// }
