#include "display.h"
#include "gps.h"
#include "logging.h"
#include "io.h"
#include <U8g2lib.h> 
#include "globalData.h"
volatile int tachPulse = 0;
volatile int pulseCount = 0;
const uint8_t numPulses = 10;
volatile unsigned long pulseTime[numPulses] = {0}; // keep track of the last 10 pulse times
volatile uint8_t pulseIndex = 0; // used to record to pulsetime
unsigned int pulseDeltaTimes[numPulses]; // keep track of the last 10 pulse times
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
  pulseTime[pulseIndex] = micros(); // note the time in microseconds when the pulse occured
  pulseIndex++;
  if(pulseIndex >= numPulses) // if the pulse index exceeds the array length
  {
    pulseIndex = 0; // reset the pulse index
  }
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
  initializeDisplay();
  setSyncProvider(getTeensy3Time);
  Serial.begin(9600);
  Serial.println("test");
  //checkPulses.priority(40);
  // Serial.println(modf(10.51234512,1.0);
  pinMode(LED_BUILTIN,OUTPUT);
  initializeIO();
  initializeGPS();
  initializeSD();
  pinMode(32,INPUT);
  attachInterrupt(32,tachPulseEvent,FALLING);
  checkPulses.begin(pulseTally,50000);
  //initializeEaganM3_Screen();
  //gpsSpeed = 0;
}

void loop() {
  if(lastLogEntry > 50)
  {
    readIO();
    logData();
    lastLogEntry = 0;
  }
  updateGPS();
  //Serial.println(displayUpdateTime);
  unsigned long pulseDeltaAvg = 0;
  noInterrupts();
  for(uint8_t i = 1; i < numPulses; i++)
  {
    unsigned int pulseDelta = abs(pulseTime[i] - pulseTime[i-1]); // find the diffrence in time between the two pulses
    
    //pulseDelta = pulseDelta / 100; // devide by 100 to get int milliseconds
    Serial.println(pulseDelta);
    pulseDeltaAvg = pulseDelta + pulseDeltaAvg; // add the pulse delta to the running average
  }
  interrupts();
  Serial.println(pulseDeltaAvg);
  pulseDeltaAvg = pulseDeltaAvg / (numPulses - 1); 
  Serial.println(pulseDeltaAvg);


  int newtachFreq = (pulseCount * 500) / 50; // measure the frequency of the tach wire (change interupt means two counts per pulse)
   if (abs(newtachFreq - tachFreq) <= 10){ // if the diffrence between the frequencies is small
    tachFreq = max(tachFreq,newtachFreq); // use the larger of the two calculated frequencies to prevent jitter
   }
   else
   { // use the new frequency
    tachFreq = newtachFreq;
   }
  // Serial.print(pulseCount);
  // Serial.print("  ");
  // Serial.print(pulseInterval);
  // Serial.print("  ");
  // Serial.println(tachFreq);
  engRPM = tachFreq * pulsePerRPM;

//EaganM3_Screen();
displayScreen();
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

