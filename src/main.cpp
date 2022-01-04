#include "display.h"
#include "gps.h"
#include "logging.h"
#include "io.h"
#include <U8g2lib.h> 
#include "globalData.h"

elapsedMillis pulseUpdate;
elapsedMillis displayUpdateTime;
elapsedMillis lastLogEntry;
IntervalTimer checkPulses;


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
  
  //checkPulses.begin(pulseTally,50000);
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

