#include "display.h"
#include "gps.h"
#include "logging.h"
#include <U8g2lib.h> 
#include "globalData.h"
volatile int tachPulse = 0;
volatile int pulseCount = 0;
int tachCount = 0;
int pulseInterval = 100;
int tachFreq = 0;
int tachRpm = 0;
elapsedMillis pulseUpdate;
elapsedMillis displayUpdateTime;
elapsedMillis lastLogEntry;
IntervalTimer checkPulses;
void tachPulseEvent()
{
  tachPulse++;
}
void pulseTally()
{
  noInterrupts();
  pulseCount = tachPulse;
  tachPulse = 0;
  interrupts();
}

void setup() {
  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(100);
  // digitalWrite(LED_BUILTIN, LOW);
  // delay(100);
  // drawBoxGauge(8000,12000,cutoff);
  // circularGaugeLayout();
  initializeDisplay();
  setSyncProvider(getTeensy3Time);
  Serial.begin(9600);
  Serial.println("test");
  pinMode(32,INPUT);
  attachInterrupt(32,tachPulseEvent,CHANGE);
  checkPulses.begin(pulseTally,50000);
  //checkPulses.priority(40);
  // Serial.println(modf(10.51234512,1.0);
  pinMode(LED_BUILTIN,OUTPUT);
  initializeGPS();
  initializeSD();
  initializeEaganM3_Screen();
  //gpsSpeed = 0;
}

void loop() {
  if(lastLogEntry > 50)
  {
    logData();
    lastLogEntry = 0;
  }
  updateGPS();

  int newtachFreq = (pulseCount * 500) / 50; // measure the frequency of the tach wire (change interupt means two counts per pulse)
   if (abs(newtachFreq - tachFreq) <= 10){ // if the diffrence between the frequencies is small
    tachFreq = max(tachFreq,newtachFreq); // use the larger of the two calculated frequencies to prevent jitter
   }
   else
   { // use the new frequency
    tachFreq = newtachFreq;
   }
  //Serial.print(pulseCount);
  //Serial.print("  ");
  // Serial.print(pulseInterval);
  // Serial.print("  ");
  //Serial.println(tachFreq);
  engRPM = tachFreq * 20;

EaganM3_Screen(engRPM);
//Serial.println(displayUpdateTime);
displayUpdateTime = 0;

//BakerFSAEscreen();

  // put your main code here, to run repeatedly:
  // toDo have simple setup screen function to walk user through setup options
  // fetch data from diffrent sources and display it to the user on screens
  //allow user to log events or enable continous logging of all data channels
  // have performance metric modes that test 0-60, 5-60, lateral acceleration...
  //... acceleration by RPM (Dyno Mode)
  // math channels that calculate MPG, Wh/mi, BSFC (MPH/S / (cc/s,G/s)) VE? (cc/s)

}

