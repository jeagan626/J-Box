#include "display.h"
#include "gps.h"
#include <U8g2lib.h> 
volatile int tachPulse = 0;
int tachCount = 0;
int pulseInterval = 100;
int tachFreq = 0;
int tachRpm = 0;
elapsedMillis pulseUpdate;
void tachPulseEvent()
{
  tachPulse++;
}


void setup() {
  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(100);
  // digitalWrite(LED_BUILTIN, LOW);
  // delay(100);
  // drawBoxGauge(8000,12000,cutoff);
  // circularGaugeLayout();
  initializeDisplay();
  Serial.begin(9600);
  Serial.println("test");
  attachInterrupt(32,tachPulseEvent,CHANGE);
  // Serial.println(modf(10.51234512,1.0);
  pinMode(LED_BUILTIN,OUTPUT);
  initializeEaganM3_Screen;
}

void loop() {
if(pulseUpdate > 50)
{
  noInterrupts();
  tachCount = tachPulse;
  pulseInterval = pulseUpdate;
  tachPulse = 0;
  pulseUpdate = 0;
  interrupts();
  int newtachFreq = (tachCount * 500) / pulseInterval; // measure the frequency of the tach wire (change interupt means two counts per pulse)
   if (abs(newtachFreq - tachFreq) <= 10){ // if the diffrence between the frequencies is small
   tachFreq = max(tachFreq,newtachFreq); // use the larger of the two calculated frequencies to prevent jitter
   }else{ // use the new frequency
  tachFreq = newtachFreq;
  }
  //tachFreq = tachCount * (1000.0 / pulseUpdate);
  Serial.print(tachCount);
  Serial.print("  ");
  Serial.print(pulseInterval);
  Serial.print("  ");
  Serial.println(tachFreq);
  tachRpm = tachFreq * 20;
  //drawBoxGauge(tachRpm, 8500,2000,7500);
}

//BakerFSAEscreen();

  // put your main code here, to run repeatedly:
  // toDo have simple setup screen function to walk user through setup options
  // fetch data from diffrent sources and display it to the user on screens
  //allow user to log events or enable continous logging of all data channels
  // have performance metric modes that test 0-60, 5-60, lateral acceleration...
  //... acceleration by RPM (Dyno Mode)
  // math channels that calculate MPG, Wh/mi, BSFC (MPH/S / (cc/s,G/s)) VE? (cc/s)

}

