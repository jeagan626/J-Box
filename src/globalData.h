#ifndef GLOBALDATA_H
#define GLOBALDATA_H
#include <Arduino.h>
// idea: put all the data shared by all the files here
// this way if some files are not called or included in a particular build there arent any interdependancies
// since these values could be derived many ways
extern int engRPM;
extern int throttlePosition;
extern int MAP;
extern int intakeAirTemp;
extern int oilPressure;
extern int fuelPressure;
extern int knockValue;
extern int maxKnockValue;
extern int AirFuelRatio;

extern int hybridBatteryVoltage;
extern int engLoad;
extern int hybridBatteryCurrent;
extern int hybridBatteryTemp;
extern int hybridBatteryCharge;
extern int ecuAFR;
extern int ecuMAP;
extern int ecuTiming;

extern int rawEcuMapReading;
extern int rawEcuIatReading;

extern unsigned int gpsUpdateTime;
extern unsigned int serialExtractTime;
extern unsigned int ioReadTime;
extern unsigned int dataLogTime;
extern unsigned int displayUpdateTime;
extern unsigned int mainLoopTime;

extern int rpmPerPulse;
extern bool loggingActive;
extern bool loggingSuccessful;
enum loggingState {logRunning,sdError,dirError,fileError,loggingOff};
extern loggingState loggingStatus;


#endif