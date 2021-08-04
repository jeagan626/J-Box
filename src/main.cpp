#include <Arduino.h>
#include <U8g2lib.h>
#include "TouchScreen.h"
#include <Wire.h> //Needed for I2C to GPS
#include "SparkFun_u-blox_GNSS_Arduino_Library.h" //http://librarymanager/All#SparkFun_u-blox_GNSS

SFE_UBLOX_GNSS myGNSS;
U8G2_T6963_240X128_F_8080 u8g2(U8G2_R2, 2, 14, 7, 8, 6, 20, 21, 5, /*enable/wr=*/ 27 , /*cs/ce=*/ 26, /*dc=*/ 25, /*reset=*/24); // Connect RD (orange) with +5V, FS0 and FS1 with GND


#define YP A16  // must be an analog pin, use "An" notation!
#define XM A17  // must be an analog pin, use "An" notation!
#define YM A14  // can be a digital pin
#define XP A15   // can be a digital pin
#define MINPRESSURE 10
#define MAXPRESSURE 1000
TSPoint p;
void modifyPointToScreen()
{
p.x =  map(p.x,100,944,0,240);
p.y =  map(p.y,190,860,0,128);
p.z = abs(map(p.z,900,300,0,255));
}
TouchScreen ts = TouchScreen(XP, YP, XM, YM , 730);
enum screens {menu,setupScreen,gps,naughtTo60Timer};
screens currentScreen = setupScreen;
void menuScreen();
void setupScreen();
void gpsScreen();
void naughtTo60Screen();



void setup() {
  // put your setup code here, to run once:
u8g2.begin();
  //u8g2.setFlipMode(0);
  u8g2.setContrast(255);
  Wire.begin();
  Wire.setClock(40000);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(32,20,"Booting Up");
  u8g2.sendBuffer();


}

void loop() {
  // put your main code here, to run repeatedly:
  // toDo have simple setup screen function to walk user through setup options
  // fetch data from diffrent sources and display it to the user on screens
  //allow user to log events or enable continous logging of all data channels
  // have performance metric modes that test 0-60, 5-60, lateral acceleration...
  //... acceleration by RPM (Dyno Mode)
  // math channels that calculate MPG, Wh/mi, BSFC (MPH/S / (cc/s,G/s)) VE? (cc/s)

}

