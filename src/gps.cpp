#include "gps.h"
#include "display.h"
SFE_UBLOX_GNSS myGNSS;
float latitude = 0;
float longitude = 0;
float gpsSpeed = 0;
int xAccel = 0;
int yAccel = 0;
int zAccel = 0;
int gpsUpdateEvent = 0;
int gpsRealUpdateRate = 0;
int bestGpsUpdateRate = 0;
int executionCounter = 0;
int executionRate = 0;
bool isGpsCalibrated = false; // use these flags to avoid wasting time
bool usingAutoHNRPVT = false;
bool usingAutoHNRDyn = false;
bool usingAutoHNRAtt = false;
bool GPSconnected = false;
bool performanceState = false;
bool newUpdate = false;

void getHNRINSdata(UBX_HNR_INS_data_t ubxDataStruct)
{
  xAccel = ubxDataStruct.xAccel;
  yAccel = ubxDataStruct.yAccel;
  newUpdate = true;
}

void getHNRPVTdata(UBX_HNR_PVT_data_t ubxDataStruct)
{
  latitude = ubxDataStruct.lat / 10000000.0;
  longitude = ubxDataStruct.lon / 10000000.0;
  gpsSpeed = ubxDataStruct.gSpeed * (2.23694 / 1000);
  gpsUpdateEvent++;
  newUpdate = true;
}

int initializeGPS()
{
    int error = 0;
    Wire.begin();
    
    Wire.setClock(40000);
    if(myGNSS.begin() == false) //Connect to the Ublox module using Wire port
  {
    GPSconnected = false;
    return(0);
  }
    GPSconnected = true;
    error++; // increment the error message to register the task as succesfull
    myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
    myGNSS.setNavigationFrequency(1); // set the internal GPS frequency at 1hz per datasheet recommendations
    myGNSS.setMeasurementRate(1); // see above
     if (myGNSS.setHNRNavigationRate(20) == false) //Set the High Navigation Rate 
      error+=2;
    usingAutoHNRDyn = myGNSS.setAutoHNRINS(true); //Attempt to enable auto HNR vehicle dynamics messages
    if (myGNSS.setAutoHNRINScallback(&getHNRINSdata) == false) // Enable automatic HNR INS messages with callback to printHNRINSdata
      error+=4; 
    if (myGNSS.setAutoHNRPVTcallback(&getHNRPVTdata) == false) // Enable automatic HNR PVT messages with callback to printHNRPVTdata
      error+=8;
    myGNSS.setI2CpollingWait(5);
    return(error); // return the correct error message based on the number of succesfully completed tasks
    // note this may not give accurate error messages;
    // error messages are encoded in binary
}

void updateGPS()
{
  myGNSS.checkUblox(); // Check for the arrival of new data and process it.
  myGNSS.checkCallbacks(); // Check if any callbacks are waiting to be processed.
}