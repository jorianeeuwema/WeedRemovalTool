#include "mbed.h"
#include "Servo/Servo.h"

enum Onkruidstatus { //enum met verschillende states waarin het programma zich kan bevinden.
  initial,
  onkruid_oppakken,
  onkruid_error,
  onkruid_loslaten
};

AnalogIn sensorValue(A1);
float voltage = sensorValue * (3.3 / 1023.0);

bool Ir_sensor(){ //De irsensor wordt uitgelezen en een true of false waarde wordt aan het voltage gehangen.
  if (voltage < 0.5){
    return false; 
  }
  else {
    return true;
  } 
}

  DigitalIn startknop(D2);//startknop aangesloten
  DigitalIn resetknop(D3); //resetknop aangesloten
  DigitalIn noodknop(D4); //noodknop aangesloten.

  DigitalOut rgbLedRood1(D9);
  DigitalOut rgbLedGroen1(D10); 
  DigitalOut rgbLedBlauw1(D8);

  DigitalOut rgbLedRood2(D12);
  DigitalOut rgbLedGroen2(D13);
  DigitalOut rgbLedBlauw2(D11);

  PwmOut lineaireMotor(D5);
  Servo servoMotor(D6);

//Verschillende manieren van hoe de leds kunnen branden. Deze worden gebruikt in de casestructure.
void Led_status_initial() {
  rgbLedRood1 = false;
  rgbLedRood2 = false;
  rgbLedGroen1 = false;
  rgbLedGroen2 = false;
  rgbLedBlauw1 = true;
  rgbLedBlauw2 = false;
}

void Led_status_onkruid_oppakken() {
  rgbLedRood1 = false;
  rgbLedRood2 = false;
  rgbLedGroen1 = true;
  rgbLedGroen2 = false;
  rgbLedBlauw1 = false;
  rgbLedBlauw2 = false;
}

void Led_status_onkruid_loslaten() {
  rgbLedRood1 = false;
  rgbLedRood2 = false;
  rgbLedGroen1 = true;
  rgbLedGroen2 = true;
  rgbLedBlauw1 = false;
  rgbLedBlauw2 = true;
}

void Led_status_onkruid_error() {
  rgbLedRood1 = true;
  rgbLedRood2 = true;
  rgbLedGroen1 = false;
  rgbLedGroen2 = false;
  rgbLedBlauw1 = false;
  rgbLedBlauw2 = false;
}


  
int main() {

  Onkruidstatus machine1 = Onkruidstatus::initial; //Maak een Onkruidstatus (enum) aan en zet hem op de basis case
 
  Timer error_timer; //timer indien na x seconden het onkruid niet is gedetecteerd, dan schiet de machine in error modus.
  Timer brandtijd_timer; //time die de brandtijd van de led bijhoudt.

  bool entry_machine1 = true;

  while (true) {
    switch (machine1) {
      case initial:
        if (entry_machine1 == true) {
          Led_status_initial();
          lineaireMotor.write(1.0f); //lineaire motor gaat volledig uit, de bek gaat open.
          servoMotor.write(1.0f); //Servomotor gaat volledig links om, de IR-sensor zit niet op de klem heen.
          entry_machine1 = false;
        }
        if (startknop == true) {
          machine1 = Onkruidstatus::onkruid_oppakken;
          entry_machine1 = true;
          brandtijd_timer.start();
          error_timer.start();
        }
        break;
      case onkruid_oppakken:
        if (entry_machine1 == true) {
          Led_status_onkruid_oppakken();
          lineaireMotor.write(0.01f); //lineaire motor gaat volldig in, de bek gaat dicht.
          servoMotor.write(0.57f); //lineaire motor gaat rechts om, de IR-sensor zit om de klem heen.
          entry_machine1 = false;
        }
        if (error_timer.read_ms() > 10000) {
          machine1 = Onkruidstatus::onkruid_error;
          brandtijd_timer.stop();
          brandtijd_timer.reset();
          error_timer.stop();
          error_timer.reset();
          entry_machine1 = true;
        }
        if (lineaireMotor.read() == 0.01f && brandtijd_timer.read_ms() > 2000 || Ir_sensor() == true) {
          machine1 = Onkruidstatus::onkruid_loslaten;
          brandtijd_timer.stop();
          brandtijd_timer.reset();
          error_timer.stop();
          error_timer.reset();
          entry_machine1 = true;
        }
        break;
      case onkruid_error:
        if (entry_machine1 == true) {
          Led_status_onkruid_error();
          entry_machine1 = false;
        }
        if (resetknop == true) {
          machine1 = Onkruidstatus::initial;
          entry_machine1 = true;
        }
        break;
      case onkruid_loslaten:
        if (entry_machine1 == true) {
          Led_status_onkruid_loslaten();
          lineaireMotor.write(1.0f);
          servoMotor.write(1.0f);
          entry_machine1 = false;
        }
        if (startknop == true) {
          machine1 = Onkruidstatus::onkruid_oppakken;
          entry_machine1 = true;
          brandtijd_timer.start();
          error_timer.start();
        }
        if (resetknop == true || lineaireMotor.read() == 1.0f){ 
          machine1 = Onkruidstatus::initial;
          entry_machine1 = true;
        }
        break;
      default:
        break;
    }
  }
}