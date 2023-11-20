#include "io.h"
#include "globalData.h"
#include <SoftPWM.h>
#define IIR_FILTER(input, alpha, prior) (((long)input * (256 - alpha) + ((long)prior * alpha))) >> 8

IntervalTimer sampleTimer;
volatile int tachPulse = 0;
volatile int pulseCount = 0;
const uint8_t numPulses = 5;
const int sampleFrequency = 240;
const int numSamples = 8;
volatile uint8_t sampleIndex = 0; // used to record to samples
volatile int sampleBuffer[8][numSamples] = {0}; // sample buffer holds the samples for each of the analog inputs

volatile unsigned long pulseTime[numPulses] = {0}; // keep track of the last 10 pulse times
volatile uint8_t pulseIndex = 0; // used to record to pulsetime
unsigned int pulseDeltaTimes[numPulses]; // keep track of the last 10 pulse times
unsigned long pulseSum = 0;
int tachCount = 0;
int pulseInterval = 100;
int tachFreq = 0;
int tachRpm = 0;

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


const int ecuNUMprams = 8;
int *ecuData[ecuNUMprams] = // Parameters pulled from OBDII C&C (must be in same order as device)
{&hybridBatteryVoltage,&knockValue,
&hybridBatteryCurrent,&intakeAirTemp,
&hybridBatteryTemp,&ecuTiming,
&hybridBatteryCharge,&ecuAFR};

const int ecuMapPin = A12;
const int turbinePressurePin = A13;
const int lambdaPin = A14;
const int tpsPin = A15;
const int mapPin = A16;
const int iatPin = A17;
const int oilPressSensorPin = A18;
const int fuelPressSensorPin = A19;
const int auxLSpin = 30;
const int tachPin = 28;

float OilPressureConvert(int ADCval);
float FuelPressureConvert(int ADCval);
float TurbinePressureConvert(int ADCval);
float MAPConvert(int ADCval);
float MAP2Boost(float myMAP);
float getFuelEnrichment(float boostPressure, float fuelPressure);
float getFmuGain(float boostPressure, float fuelPressure);
int lambdaConvert(int ADCval);
void initializeIO()
{
    pinMode(ecuMapPin,INPUT);
    pinMode(turbinePressurePin,INPUT);
    pinMode(lambdaPin,INPUT);
    pinMode(tpsPin,INPUT);
    pinMode(mapPin,INPUT);
    pinMode(iatPin,INPUT);
    pinMode(oilPressSensorPin,INPUT);
    pinMode(fuelPressSensorPin,INPUT);
    pinMode(tachPin,INPUT_PULLUP);
    attachInterrupt(tachPin,tachPulseEvent,FALLING);
    sampleTimer.begin(readIO,(1000000 / sampleFrequency));
    pinMode(0,INPUT);
    pinMode(auxLSpin,OUTPUT);
    analogWriteFrequency(auxLSpin,25); // set the PWM frequency on pin 30 to 25hz
    digitalWrite(auxLSpin,LOW); // make sure this is low for now
    //Serial1.begin(9600); // for old OBii C&C software
    Serial1.begin(38400,SERIAL_8N1_RXINV); // for new obdII C&C software


}

void readIO()
{ // cleanup sample buffer could recognize 
  
    sampleBuffer[0][sampleIndex] = analogRead(oilPressSensorPin);
    sampleBuffer[1][sampleIndex] = analogRead(mapPin);
    sampleBuffer[2][sampleIndex] = analogRead(ecuMapPin);
    sampleBuffer[3][sampleIndex] = analogRead(iatPin);
    sampleBuffer[4][sampleIndex] = analogRead(fuelPressSensorPin);
    sampleBuffer[5][sampleIndex] = analogRead(tpsPin);
    sampleBuffer[6][sampleIndex] = analogRead(lambdaPin);
    sampleBuffer[7][sampleIndex] = analogRead(turbinePressurePin);
    
    sampleIndex++;
    if(sampleIndex >= numSamples) // if the sample index exceeds the array length
    {
      sampleIndex = 0; // reset the sample index
    }
}
void processIO()
{
    oilPressure = OilPressureConvert(averageSamples(sampleBuffer[0]));
    MAP = MAPConvert(averageSamples(sampleBuffer[1]));
    rawEcuMapReading = averageSamples(sampleBuffer[2]);
    rawEcuIatReading = averageSamples(sampleBuffer[3]);
    fuelPressure = FuelPressureConvert(averageSamples(sampleBuffer[4]));
    readTach();
    throttlePosition = map(averageSamples(sampleBuffer[5]),110,920,0,99);
    AirFuelRatio = lambdaConvert(averageSamples(sampleBuffer[6]));
    turbinePressure = TurbinePressureConvert(averageSamples(sampleBuffer[7]));
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
void dumbBoostControl()
{
  if( MAP < 150 && engRPM > 2000 && throttlePosition > 60)
  {
    analogWrite(auxLSpin,50);
    //digitalWrite(auxLSpin,HIGH);
  }
  else
  {
    //analogWrite(auxLSpin,50);
    digitalWrite(auxLSpin,LOW);
    //digitalWrite(auxLSpin,HIGH);
  }
}
float OilPressureConvert(int ADCval)
{
  // this is for the 100PSI sensor
  if(ADCval > 1015)
  return(999);
  if (ADCval <= 101)
  {
    return (0);
  }
  float OilPressure = (.1272 * ADCval) - 12.068;
  return OilPressure;
}

float FuelPressureConvert(int ADCval)
{
  // this is for the 200PSI sensor
  if(ADCval > 1015)
  return(999);
  if (ADCval <= 101)
  {
    return (0);
  }
  float FuelPressure = (.2481 * ADCval) - 23.196;
  return FuelPressure;
}

float TurbinePressureConvert(int ADCval)
{
  // // this is for the 30PSI sensor
  // if(ADCval > 1015)
  // return(999);
  // if (ADCval <= 101)
  // {
  //   return (0);
  // }
  // float TurbinePressure = (.0376 * ADCval) - 2.781;
  float TurbinePressure = map(ADCval,102,920,0,100); // use Kpa 14.7Psi = 100kpa
  TurbinePressure = constrain(TurbinePressure,0,400);
  return TurbinePressure;
}

float MAPConvert(int ADCval)
{
  if(ADCval > 1015)
  {
  return(999);
  }
  float MAP = (float(ADCval) + 3.611) / 3.253;
  return MAP;
}

float MAP2Boost(float myMAP)

{
  float boost = ((myMAP - 101.325) / 101.325) * 14.7;
  return boost;
}

float getFuelEnrichment(float boostPressure, float fuelPressure)
{
  float fuelEnrichment = (.1490712 * sqrt(fuelPressure - boostPressure)) 
  / ((boostPressure + 14.7) / 14.7);
  return fuelEnrichment;
}

float getFmuGain(float boostPressure, float fuelPressure)
{
  if (boostPressure > 0)
  {
    float fmuGain = ((fuelPressure - 45) / boostPressure) - 1;
    fmuGain = constrain(fmuGain,0,50);
    return fmuGain;
  }
  else
  {
    return 0;
  }
}

int averageSamples(volatile int n1[]) 
{
  
  double sum = 0; // to store sum value
  // calculate sum value
  //noInterrupts(); // stop interupts to capture all the samples
  for (int i = 0; i < numSamples; ++i)
  {
    sum += n1[i];
  }
  //interrupts(); // return normal interupt function
  // calculate average value
  // and return to caller function
  return sum/numSamples;
}

int lambdaConvert(int ADCval)
{
  int tempLambda = map(ADCval,0,1023,735,2239);
  return(tempLambda);
}
void extractSerialData() // extracts data from the serial data stream from the obdii C&C
{
  #define maxBufferSz 120
  #define maxDataElements 16
  //int *ecuData[] = {&vehicleSpeed,&engRPM,&throttlePosition,&ecuMAP,&fuelInjectionPulseWidth,&AirFuelRatio,&intakeAirTemp,&knockValue};
  if ((Serial1.available() > 40) && (Serial1.available() < 120))
  {
    char readBuffer[maxBufferSz + 1]; // store the information here and leave room for a termination char
    uint8_t bufferSize = Serial1.readBytesUntil('\r',readBuffer,maxBufferSz); // keep track of the buffer size for later
    readBuffer[bufferSize] = '\0'; // terminate the used portion of the read buffer with a null
    uint8_t bufferIndex = 0;
    uint8_t dataIndex = 0;
    int extractedData[maxDataElements];
    //Serial.println(readBuffer);
    while(bufferSize > bufferIndex)
    {
      if(readBuffer[bufferIndex] == ',') // if we find a comma
      {
        if (readBuffer[bufferIndex + 1] == ' ' || readBuffer[bufferIndex + 1] == '+' || readBuffer[bufferIndex + 1] == '-') 
        // and the next charactor is a space or a plus or a minus
        {
          // prepare an array of digits in anticipation for the number that may follow
          char myDigits[10] = "         ";
          myDigits[0] = readBuffer[bufferIndex + 1]; // store the preceding sign into the first place in my digits
          uint8_t digitIndex = 1;
           if(readBuffer[bufferIndex + 2] == ',')
           // and the charactor after that is another comma
           {
              bufferIndex += 3; // add 3 to the buffer index since we've alreadly looked ahead 2 charactors
              // at this point we have matched to our pattern
              while (isdigit(readBuffer[bufferIndex])) 
              // now start reading data until we hit another comma or some non numeric data
              {
                myDigits[digitIndex] = readBuffer[bufferIndex]; // copy the digit in the buffer to one of mydigits
                digitIndex++;
                bufferIndex++; // move on to the next digit in the buffer
              }
              if(digitIndex > 1) // check to make sure it actually captured some data before saving anything
              {
                extractedData[dataIndex] = atoi(myDigits); //convert my string of digits to a real number
                dataIndex++; // advance to the next piece of data
              }
              continue; // go to the next interation of the main loop since we've already found the next comma
              // and the current buffer index is now returning a non numeric charactor
           }
        }
      }
      // we didn't find a match to our pattern so... 
      bufferIndex++; // move on to the next charactor in the buffer
    }
    if(dataIndex == ecuNUMprams) 
    // if the number of data blocks we extracted excatly matches the number of paramaters we expect
    {
      // we've got good data so now save it to the appropriate variables
      for(dataIndex = 0; dataIndex < ecuNUMprams; dataIndex++ )
      {
        *ecuData[dataIndex] = extractedData[dataIndex];
        // *ecuData points to the individual ecu paramater variables
        // in the exact order they are relayed over serial
        // so simply copy these extracted data blocks sequentially to the variables refrenced in ecuData
        //Serial.println(*ecuData[dataIndex]); // print the extracted data to the console for debugging
      }
    }
    //writeData();
  }
}