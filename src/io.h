#ifndef IO_H
#define IO_H

void initializeIO();
void readIO();
void processIO();
void extractSerialData();
void readTach();
void readFlexSensor();
void dumbBoostControl();
int averageSamples(volatile int n1[]);
#endif