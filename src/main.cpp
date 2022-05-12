#include "display.h"
#include "gps.h"
#include "logging.h"
#include "io.h"
#include <U8g2lib.h> 
#include "globalData.h"

elapsedMicros actionTime;
elapsedMicros loopTime;
elapsedMillis lastLogEntry;
void setup() {
  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(100);
  // digitalWrite(LED_BUILTIN, LOW);
  // delay(100);
  initializeDisplay();
  initializeSysClock();
  Serial.begin(115200);
  Serial.println("test");
  // Serial.println(modf(10.51234512,1.0);
  pinMode(LED_BUILTIN,OUTPUT);
  initializeIO();
  initializeGPS();
  initializeSD();
}

void loop() {
  mainLoopTime = loopTime;
  loopTime = 0;
  actionTime = 0;
  extractSerialData();
  serialExtractTime = actionTime; // note the time taken to extract the serial data
  
  actionTime = 0;
  updateGPS();
  gpsUpdateTime = actionTime;

  if(lastLogEntry > 40)
  {
    actionTime = 0;
    processIO();
    ioReadTime = actionTime; // note the time taken to extract the serial data
    actionTime = 0;
    logData();
    dataLogTime = actionTime;
    
    lastLogEntry = 0;
  }
  dumbBoostControl();
  actionTime = 0;
  displayScreen();
  displayUpdateTime = actionTime;
  // Serial.print(serialExtractTime);
  // Serial.print(',');
  // Serial.print(gpsUpdateTime);
  // Serial.print(',');
  // Serial.print(ioReadTime);
  // Serial.print(',');
  // Serial.print(dataLogTime);
  // Serial.print(',');
  // Serial.print(displayUpdateTime);
  // Serial.print(',');
  // Serial.println(loopTime);

//BakerFSAEscreen();

  // put your main code here, to run repeatedly:
  // toDo have simple setup screen function to walk user through setup options
  // fetch data from diffrent sources and display it to the user on screens
  //allow user to log events or enable continous logging of all data channels
  // have performance metric modes that test 0-60, 5-60, lateral acceleration...
  //... acceleration by RPM (Dyno Mode)
  // math channels that calculate MPG, Wh/mi, BSFC (MPH/S / (cc/s,G/s)) VE? (cc/s)

}

