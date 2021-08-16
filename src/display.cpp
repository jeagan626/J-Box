#include "display.h"

U8G2_T6963_240X128_F_8080 u8g2(U8G2_R2, 2, 14, 7, 8, 6, 20, 21, 5, /*enable/wr=*/ 27 , /*cs/ce=*/ 26, /*dc=*/ 25, /*reset=*/24); // Connect RD (orange) with +5V, FS0 and FS1 with GND
TSPoint p;
void modifyPointToScreen()
{
p.x =  map(p.x,100,944,0,240);
p.y =  map(p.y,190,860,0,128);
p.z = abs(map(p.z,900,300,0,255));
}
TouchScreen ts = TouchScreen(XP, YP, XM, YM , 730);
enum screens {menu,settings,gps,naughtTo60Timer};
screens currentScreen = settings;
void menuScreen();
void setupScreen();
void gpsScreen();
void naughtTo60Screen();

void initialiseDisplay()
{
    pinMode(backlightPin,OUTPUT);
    analogWrite(backlightPin,100);
    u8g2.begin();
    //u8g2.setFlipMode(0);
    u8g2.setContrast(255);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(32,20,"Booting Up");
    u8g2.sendBuffer();
}
// void displayGPSbootup()
// {
//     u8g2.setCursor(32,40);
//     u8g2.print("gps not functional");

// }