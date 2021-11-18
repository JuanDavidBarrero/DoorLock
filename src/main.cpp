#include <Arduino.h>
#include <Finger_Sensor.h>
#include <EasyNextionLibrary.h>

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#define NextionScreen Serial1
EasyNex myNex(NextionScreen);

FingerSensor Sensor;
int personID = -1;
bool timeOut = false;
String family[127];

void IRAM_ATTR onTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);
  timeOut = true;
  timerWrite(timer, 0);
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup()
{
  Serial.begin(115200);
  myNex.begin(9600);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 10000000, false);

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
      timeOut=false;
      return;
    }
  }

  myNex.writeStr("page 1");
  myNex.writeStr("t1.txt", family[personID - 1]);
  personID = -1;
  delay(3000);
  myNex.writeStr("page 0");
}

void trigger1()
{
  String nombre = myNex.readStr("t1.txt");
  int id = myNex.readNumber("n0.val");
  if (id == 0 || id > 127)
    return;
  myNex.writeStr("page 5");
  Sensor.enrollFinger(id);
  myNex.writeStr("page 6");
  delay(1000);
  myNex.writeStr("page 5");
  Sensor.verifyFinger(id);
  family[id - 1] = nombre;
  myNex.writeStr("page 7");
  delay(1500);
  myNex.writeStr("page 2");
  // Serial.printf("El nombre enviado fue %s\n", nombre);
}

// void trigger2()
// {
//   Serial.printf("id para ser eliminado\n");
//   int id = myNex.readNumber("t1.pco");
//   Serial.printf("El nombre enviado fue %i\n", id);
// }
