#ifndef DISPLAY_H
#define DISPLAY_H
#include <Arduino.h>
#include <U8g2lib.h>
#include "TouchScreen.h"
void menuScreen();
void setupScreen();
void gpsScreen();
void circularGaugeLayout();
void naughtTo60Screen();
void initializeDisplay();
void initializeBakerFSAEscreen();
void BakerFSAEscreen();
void initializeEaganM3_Screen(int myRPM = 0);
void EaganM3_Screen();
void initializeEaganInsightScreen();
void drawMph(int mph);
void drawRpm(int rpm);
void drawBoxGauge(int current, int max, int cutoff = 4000, int redLine = 10000);
void drawGear(String gear);
void drawLapTime(int *lapTime);
void drawCoolantTemp(int coolantTemp);
void drawEngineTemp(int engineTemp);
void drawFuel(int fuelLevel);
void drawVoltage(double battVoltage);
void drawBackground2();
#define YP A3  // must be an analog pin, use "An" notation!
#define XM A8  // must be an analog pin, use "An" notation!
#define YM A1  // can be a digital pin
#define XP A2   // can be a digital pin
const int backlightPin = 23;
#define MINPRESSURE 10
#define MAXPRESSURE 1000
#endif