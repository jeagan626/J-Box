#include "SparkFun_u-blox_GNSS_Arduino_Library.h" //http://librarymanager/All#SparkFun_u-blox_GNSS
#include <U8g2lib.h>
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
void setupGPS()
{
    if(myGNSS.begin() == false) //Connect to the Ublox module using Wire port
  {
    u8g2.setCursor(32,40);
    u8g2.print("gps not functional");
  }
  else
  {
    u8g2.setCursor(32,40);
    u8g2.print("GPS connected!");
    GPSconnected = true;
  }

}
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