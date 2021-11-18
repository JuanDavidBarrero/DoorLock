#include <Arduino.h>
#include <Finger_Sensor.h>
#include <EasyNextionLibrary.h>

#define NextionScreen Serial1
EasyNex myNex(NextionScreen);

FingerSensor Sensor;
int personID = -1;
unsigned long timePage = 0;
unsigned long prevtime = 0;
unsigned long interval = 10000;
String family[127];

void setup()
{
  Serial.begin(115200);
  myNex.begin(9600);
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
  myNex.writeStr("page 5");

  while (personID == -1)
  {
    personID = Sensor.identifingFinger();
    timePage = millis();
    if (timePage - prevtime > interval)
    {
      prevtime = millis();
      myNex.writeStr("page 9");
      delay(1000);
      myNex.writeStr("page 0");
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
  if(id == 0) return;
  myNex.writeStr("page 5");
  Sensor.enrollFinger(id);
  myNex.writeStr("page 6");
  delay(500);
  myNex.writeStr("page 5");
  Sensor.verifyFinger(id);
  family[id-1] = nombre;
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
