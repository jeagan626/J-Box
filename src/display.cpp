#include "display.h"
#include <string>
#include <math.h>


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
}
void menuScreen()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_VCR_OSD_mf); //u8g2_font_9x15B_mr
    u8g2.drawStr(32,18,"Menu");
    u8g2.drawLine(0,19,240,19);
    ///u8g2.setFontMode(0); // shoulnt need to change this as it is the default
    u8g2.setDrawColor(0); // set this to zero so that we get a black background against the font
    u8g2.drawStr(0,40,"Device Settings");
    u8g2.drawStr(0,60,"");
    u8g2.drawStr(0,80,"Acceleration Tests");
    

    
    
    
    u8g2.sendBuffer();
    u8g2.setDrawColor(1); // reset the draw color back to the default




}

void drawPartialCircle(int xo, int yo, double radius, double initialAngle, double finalAngle){
    for (double angle = initialAngle; angle<=finalAngle; angle = angle + .01){
        u8g2.drawPixel(xo+int(radius*cos(angle)),yo+int(radius*sin(angle)));
    }
}

void drawGauge(int xo, int yo, int rad, double maxValue, double currentValue) {
    drawPartialCircle(xo,yo,rad*1.0,M_PI*(3.0/4.0),M_PI*(9.0/4.0));
    double angle = (maxValue/currentValue)*(3.0/2.0)*M_PI+M_PI*(3.0/4.0);
    u8g2.drawLine(xo,yo,xo+int(rad*cos(angle)),yo+int(rad*sin(angle)));
}

int mph = 99;
String gear = "N";
int rpm = 12500;
int coolantTemp = 109;
int engineTemp = 120;
int fuelLevel = 100;
int lapTime[3] = {1,23,45};
int screenx = 240;
int screeny = 128;
void enduranceLayout() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_VCR_OSD_mf); //u8g2_font_9x15B_mr
    int fontx = 12;
    int fonty = 15;
    u8g2.drawStr(0,fonty,String(mph).c_str());
    u8g2.drawStr(fontx*2,fonty,"mph");
    u8g2.drawStr(100,fonty,gear.c_str());
    u8g2.drawStr(140,fonty,String(rpm).c_str());
    u8g2.drawStr(screenx - fontx*3,fonty,"rpm");
    u8g2.drawStr(0,screeny-fonty,String(coolantTemp).c_str());
    u8g2.drawStr(fontx*3,screeny-fonty,"C");
    u8g2.drawStr(0,screeny,String(engineTemp).c_str());
    u8g2.drawStr(fontx*3,screeny,"C");
    u8g2.drawStr((screenx/2)-fontx*2,screeny,String(fuelLevel).c_str());
    u8g2.drawStr((screenx/2)+fontx,screeny,"%");
    u8g2.drawStr(screenx-fontx*8,screeny,String(lapTime[0]).c_str());
    u8g2.drawStr(screenx-fontx*6,screeny,":");
    u8g2.drawStr(screenx-fontx*5,screeny,String(lapTime[1]).c_str());
    u8g2.drawStr(screenx-fontx*3,screeny,":");
    u8g2.drawStr(screenx-fontx*2,screeny,String(lapTime[2]).c_str());
    drawGauge(screenx/2,screeny/2,50,rpm*3.0,rpm*1.0);
    u8g2.sendBuffer();
}


// void displayGPSbootup()
// {
//     u8g2.setCursor(32,40);
//     u8g2.print("gps not functional");

// }