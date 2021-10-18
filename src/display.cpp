#include "display.h"

U8G2_T6963_240X128_F_8080 u8g2(U8G2_R2, 2, 14, 7, 8, 6, 20, 21, 5, /*enable/wr=*/ 27 , /*cs/ce=*/ 26, /*dc=*/ 25, /*reset=*/24); // Connect RD (orange) with +5V, FS0 and FS1 with GND
TSPoint p;
#define MINPRESSURE 10
#define MAXPRESSURE 1000
void modifyPointToScreen()
{
p.x =  map(p.x,100,944,0,240);
p.y =  map(p.y,190,860,0,128);
p.z = abs(map(p.z,900,300,0,255));
}
TouchScreen ts = TouchScreen(XP, YP, XM, YM , 730);
enum screens {menu,settings,gps,naughtTo60Timer};
screens currentScreen = menu;
bool updateScreen = true;
void initializeDisplay()
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
    screens currentScreen = menu;
}
void menuScreen()
{
    switch (currentScreen)
    {
    case menu:
        if(updateScreen){
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_VCR_OSD_mf); //u8g2_font_9x15B_mr
            u8g2.drawStr(32,18,"Menu");
            u8g2.drawLine(0,19,240,19);
            ///u8g2.setFontMode(0); // shoulnt need to change this as it is the default
            u8g2.setDrawColor(0); // set this to zero so that we get a black background against the font
            u8g2.drawStr(0,40,"Device Settings");
            u8g2.drawStr(0,60,"Option 2");
            u8g2.drawStr(0,80,"Acceleration Tests");
            u8g2.sendBuffer();
            u8g2.setDrawColor(1); // reset the draw color back to the default
            updateScreen = false;
        }
        p = ts.getPoint();
        if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
            modifyPointToScreen();
            Serial.print(p.x);
            Serial.print(',');
            Serial.print(p.y);
            Serial.print(',');
            Serial.println(p.z);
            if(abs(p.y-30) < 10 ){
                currentScreen = settings;
                updateScreen = true;
            }
        }
        break;
        case settings:
        if(updateScreen){
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_VCR_OSD_mf); //u8g2_font_9x15B_mr
            u8g2.drawStr(5,18,"<");
            u8g2.drawStr(32,18,"Device Settings");
            u8g2.drawLine(0,19,240,19);
            ///u8g2.setFontMode(0); // shoulnt need to change this as it is the default
            u8g2.setDrawColor(0); // set this to zero so that we get a black background against the font
            u8g2.drawStr(0,40,"Calibrate Screen");
            u8g2.drawStr(0,60,"Logging Options");
            u8g2.drawStr(0,80,"COM Settings");
            u8g2.sendBuffer();
            u8g2.setDrawColor(1); // reset the draw color back to the default
            updateScreen = false;
        }
        p = ts.getPoint();
        if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
            modifyPointToScreen();
            Serial.print(p.x);
            Serial.print(',');
            Serial.print(p.y);
            Serial.print(',');
            Serial.println(p.z);
            if(abs(p.y-30) < 10 ){ //
                currentScreen = settings;
                updateScreen = true;
            }
             if(abs(p.y-5) < 8 ){ //
                currentScreen = menu;
                updateScreen = true;
            }
        }
        break;
    }
}
// void displayGPSbootup()
// {
//     u8g2.setCursor(32,40);
//     u8g2.print("gps not functional");

// }