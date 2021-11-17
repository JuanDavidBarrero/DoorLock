#include <Arduino.h>
#include <Finger_Sensor.h>
#include <EasyNextionLibrary.h>

#define NextionScreen Serial1

EasyNex myNex(NextionScreen);

FingerSensor Sensor;
int personID;

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
  Serial.printf("El boton de reconocimiento fue presionado\n");
  myNex.writeStr("page 5");
  delay(2000);
  myNex.writeStr("page 1");
  myNex.writeStr("t1.txt", "Juan David");
  delay(3000);
  myNex.writeStr("page 0");
}

void trigger1()
{
  Serial.printf("Enviando nombre\n");
  String nombre = myNex.readStr("t1.txt");
  Serial.printf("El nombre enviado fue %s\n", nombre);
}

void trigger2()
{
  Serial.printf("id para ser eliminado\n");
  int id = myNex.readNumber("t1.pco");
  Serial.printf("El nombre enviado fue %i\n", id);
}

//  personID = Sensor.identifingFinger();
//   if(personID != -1){
//     Serial.printf("The finger id correspont to %i \n", personID);
//     delay(3000);
//   }
//   Serial.print("Plese place your finger\n");