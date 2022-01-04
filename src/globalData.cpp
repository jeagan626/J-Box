#include "globalData.h"
int engRPM = 0;
int throttlePosition = 0;
int ecuMAP = 0;
int intakeAirTemp = 0;
int oilPressure = 0;
int fuelPress = 0;
int knockValue = 0;
int maxKnockValue = 0;
int AirFuelRatio = 0;

int rpmPerPulse = 20;

bool loggingActive = false;
bool loggingSuccessful = false;
loggingState loggingStatus = sdError;