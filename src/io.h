#ifndef IO_H
#define IO_H
#include "Arduino.h"

const int tachPin = A13;
const int tpsPin = A15;

void initializeIO();
void readIO();
void readTach();
#endif