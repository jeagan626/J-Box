#include "display.h"
#include "gps.h"
#include <U8g2lib.h>
volatile int tachCount = 0;
int tachFreq = 0;
int tachRpm = 0;
elapsedMillis pulseUpdate;
void tachPulse()
{
  tachCount++;
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
  attachInterrupt(32,tachPulse,FALLING);
  // Serial.println(modf(10.51234512,1.0);
  pinMode(LED_BUILTIN,OUTPUT);
  initializeBakerFSAEscreen();
}

void loop() {
if(pulseUpdate > 100)
{
  noInterrupts();
  //tachFreq = (tachCount *1000) / pulseUpdate; // measure the frequency of the tach wire
  tachFreq = tachCount * (1000.0 / pulseUpdate);
  Serial.println(tachCount);
  Serial.println(tachFreq);
  tachCount = 0;
  pulseUpdate = 0;
  interrupts();
  tachRpm = tachFreq * 20;
  drawBoxGauge(tachRpm, 8500,2000);
}

BakerFSAEscreen();

  // put your main code here, to run repeatedly:
  // toDo have simple setup screen function to walk user through setup options
  // fetch data from diffrent sources and display it to the user on screens
  //allow user to log events or enable continous logging of all data channels
  // have performance metric modes that test 0-60, 5-60, lateral acceleration...
  //... acceleration by RPM (Dyno Mode)
  // math channels that calculate MPG, Wh/mi, BSFC (MPH/S / (cc/s,G/s)) VE? (cc/s)

}

