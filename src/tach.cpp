#include "globalData.h"

volatile int tachPulse = 0;
volatile int pulseCount = 0;
const uint8_t numPulses = 5;
volatile unsigned long pulseTime[numPulses] = {0}; // keep track of the last 10 pulse times
volatile uint8_t pulseIndex = 0; // used to record to pulsetime
unsigned int pulseDeltaTimes[numPulses]; // keep track of the last 10 pulse times
unsigned long pulseSum = 0;
int tachCount = 0;
int pulseInterval = 100;
int tachFreq = 0;
int tachRpm = 0;

const int tachPin = 28;

void initializeTach()
{
    pinMode(tachPin,INPUT_PULLUP);
    attachInterrupt(tachPin,tachPulseEvent,FALLING);
}

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






uint8_t lastPulseIndex = 0;
unsigned long stalePulseTime = 1000000; // default to one second
void readTach()
{
  unsigned long pulseDeltaAvg = 0;
  noInterrupts();
  if(pulseIndex == lastPulseIndex)
  {
    // if the pulse index has not changed
    unsigned long newStalePulseTime = micros() - pulseTime[lastPulseIndex]; // record the amount of time that has passed since this last pulse
    if(newStalePulseTime > stalePulseTime)
    {
      // and the amount of time that has passed since this last pulse is greater than 1 second and then keeps increasing
      stalePulseTime = newStalePulseTime; // note the lastest time diffrence
      tachFreq = 0; // it seems we have come across a frequency less than 1hz
      engRPM = 0; // the engine rpm must also be 0
      // we are done here the tach frequency and engrpm are 0
      interrupts(); // turn the interupts back on
      return; // exit the function
    }
    // since the stale pulse time did not increase from the last recorded stale pulse time
    // it has likely been inflated by the last time
    stalePulseTime = 1000000; // reset the default to one second
  }
  lastPulseIndex = pulseIndex; // record the current pulse index so we can test the conditions above


  uint8_t numPulseDeltas = 0; // keep track of how many valid pulses deltas were found
  for(uint8_t i = 1; i < numPulses; i++)
  {
    //noInterrupts(); // we may only need to turn off interupts to read the individual deltas
    unsigned long pulseDelta = pulseTime[i] - pulseTime[i-1]; // find the diffrence in time between the two pulses
    //Serial.println(pulseDelta);
    //interrupts();
    if(pulseDelta > 10000000 ){continue;} //4,294,867,297 pulse delta shows up from time to time and is a garbage value
    // if we find it just ignore it by going to the next iteration of the loop
    // the pulse delta should never be greater than 10 seconds here
    //lastPulseDelta = pulseDelta;
    pulseSum = pulseDelta + pulseSum; // add the pulse delta to the running average
    numPulseDeltas++; // keep track of the number of deltas added to the sum so we can calculate an accurate average later
    // Serial.print(pulseSum);
    // Serial.print("  index:");
    // Serial.println(i);
  }
  //Serial.println(pulseSum);
  interrupts();
  if(numPulseDeltas == 0)
  {
    // if we find that that the pulse delta 
    tachFreq = 0;
    engRPM = 0;
    return;
  }
  pulseDeltaAvg = pulseSum / (numPulseDeltas); 
  //Serial.println(numPulseDeltas);
  pulseSum = 0;
  //Serial.println(pulseDeltaAvg);
  int newtachFreq = 1000000/pulseDeltaAvg;
  if (abs(newtachFreq - tachFreq) <= 1){ // if the diffrence between the frequencies is small
    tachFreq = max(tachFreq,newtachFreq); // use the larger of the two calculated frequencies to prevent jitter
   }
   else
   { // use the new frequency
    tachFreq = newtachFreq;
   }
  //Serial.println(tachFreq);
  engRPM = tachFreq * rpmPerPulse;
}