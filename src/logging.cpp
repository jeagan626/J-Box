#include "logging.h"
#include "gps.h"
#include "globalData.h"
char logFile[13] = "log"; //Start each logfile with "Log"
char dir[64] = "/Datalogs/"; //Store the log files in a folder called "datalogs"
char logFileDir[77] = " "; // allocate a string to lead to the datalog file we will create
bool newLog = true;
#define SD_FAT_TYPE 3
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#define SD_CONFIG SdioConfig(FIFO_SDIO)
SdFs SD;
FsFile dataFile;
elapsedMillis milliseconds;
IntervalTimer updateMillisecond;
unsigned int lastSeconds = 0;
time_t getTeensy3Time()
{
  return (Teensy3Clock.get() - (8 * 3600)); // Shift the time so it is consistant with PST
}
// String millisecond()
//   {
//     char milisString [4] = "000";
//     sprintf(milisString,"%03i",milliseconds % 1000);
//     //char *str = malloc(4); // allocate a location to pass on the string
//     //strcpy(str,milisString); // copy the desired string to pass
//     //free(str); // free the allocated space
//     return (milisString);
//   }

unsigned int rtc_ms() 
{
  // manitou48 example code for getting milisecond values from the teensy
    uint32_t read1, read2,secs,us;
 
    do{
        read1 = RTC_TSR;
        read2 = RTC_TSR;
    }while( read1 != read2);       //insure the same read twice to avoid 'glitches'
    secs = read1;
    do{
        read1 = RTC_TPR;
        read2 = RTC_TPR;
    }while( read1 != read2);       //insure the same read twice to avoid 'glitches'
//Scale 32.768KHz to microseconds, but do the math within 32bits by leaving out 2^6
// 30.51758us per TPR count
    us = (read1*(1000000UL/64)+16384/64)/(32768/64);    //Round crystal counts to microseconds
    if( us < 100 ) //if prescaler just rolled over from zero, might have just incremented seconds -- refetch
    {
        do{
            read1 = RTC_TSR;
            read2 = RTC_TSR;
        }while( read1 != read2);       //insure the same read twice to avoid 'glitches'
        secs = read1;
    }
	//return(secs*1000 + us/1000);   // ms
  return(us/1000);   // ms but just the value in between seconds
}
void millisecondUpdate() // this might not be a good idea since the loop speed may be slow enough to constantly reset miliseconds mid second
{
  // note if the time library uses millis() why cant milliseconds just be millis() mod 1000
  if(second() - lastSeconds != 0) // if one second has passed
  {
    lastSeconds = second(); // reset the counter
    if(milliseconds % 1000 != 0) // check to see if milliseconds has rolled over
     {
     // if it hasn't just rolled over then reset it
      milliseconds = 0;
    }
  } 
}
void initializeSysClock()
{
  setSyncProvider(getTeensy3Time);
  updateMillisecond.begin(millisecondUpdate,500); // update the milisecond to the nearest .5ms
}
// 3 
String constructDateTime(uint8_t i)
{ 
  char dateTimeString[64] = "";
  unsigned int myMilliseconds = milliseconds % 1000;
  switch (i)
  {
    case 0:
      sprintf(dateTimeString,"%.4i/%.2i/%.2i",year(),month(),day());
      break;

    case 1:
      sprintf(dateTimeString,"%02i_%02i",hour(),minute());
      break;
    
    case 2:
      sprintf(dateTimeString,"%.2i/%.2i/%.2i %.2i:%.2i:%.2i",month(),day(),year()%1000,hour(),minute(),second());
      break;
    
    case 3:
      //sprintf(dateTimeString,"%.2i/%.2i/%.2i %.2i:%.2i:%.2i.%.2i",month(),day(),year()%1000,hour(),minute(),second(),rtc_ms() / 10);
      sprintf(dateTimeString,"%.2i/%.2i/%.2i %.2i:%.2i:%.2i.%.2i",month(),day(),year()%1000,hour(),minute(),second(),myMilliseconds / 10);
      break;

    case 4:
      //sprintf(dateTimeString,"%.2i:%.2i:%.2i.%.3i",hour(),minute(),second(),rtc_ms());
      sprintf(dateTimeString,"%.2i:%.2i:%.2i.%.3i",hour(),minute(),second(),myMilliseconds);
      break;

    case 5:
      sprintf(dateTimeString,"%.4i/%.2i/%.2i %.2i:%.2i:%.2i",year(),month(),day(),hour(),minute(),second());
      break;
  }
  return dateTimeString;
}

void initializeSD(){ 
      if(!SD.begin(SD_CONFIG)){
        loggingStatus = sdError; // if theres an error initializing the SD card
        return;
      }
      strcpy(dir,"/Datalogs/"); // start the directory back at datlogs to avoid funky directories
      Serial.println(dir);
      strcat(dir,constructDateTime(0).c_str()); //add the current date MM-YYYY to the dir to make a dated folder within "datalogs"
      Serial.println(dir);
      if(!SD.mkdir(dir)){ // create the dated directory
        loggingStatus = dirError;
        return;
      } 
      
}

bool initializeLog()
{
  initializeSD(); // do this just for good measure
  if(loggingStatus == sdError){ // if theres an error
    return(false);
  }
  char oldLogFile [13] = "log"; // initialize a string to store the name of the old log file
  strcpy(oldLogFile,logFile); // save the name of the old logfile
  strcpy(logFile,"log"); // reset the logfile name
  strcat(logFile,constructDateTime(1).c_str()); // add the current time to the file name
  Serial.println(logFile);
  strcat(logFile,".csv");// tack on the .txt so we can open it later
  Serial.println(logFile);
  Serial.print("oldLogFile: ");
  Serial.println(oldLogFile);
  
  strcpy(logFileDir,dir);//add the current directory to the log file directory string
  Serial.println(logFileDir);
  strcat(logFileDir,"/");//add the / so we can locate our file within the current directory
  Serial.println(logFileDir);
  strcat(logFileDir,logFile); // add the log file name to the directory so we can create the file
  Serial.println(logFileDir);
  if(!dataFile.open(logFileDir, FILE_WRITE)){
    loggingStatus = fileError;
    return(false);
  }
  if(strcmp(logFile,oldLogFile) != 0)
  {
    // if the two filesnames are not the same
    // create a header file for the new log
    Serial.println("writing Header text...");
    dataFile.print("\n"); // start another line
    dataFile.print(constructDateTime(5)); // print the date and time at the top of the file
    dataFile.print("\n\n"); // start two lines down
    dataFile.println("Time,Lat,Long,Speed,Xg,Yg,RPM,TPS,rawEcuMapReading,MAP,EPress,IAT,rawEcuIatReading,KNK,ecuTiming,ecuAFR,AFR,FuelPress,HybridBatteryCharge,HybridVoltage,HybridCurrent,BatteryTemp,serialExtractTime,gpsUpdateTime,ioReadTime,dataLogTime,displayUpdateTime,loopTime");
              /* Preview of the data writing process
              char dataString [128] = "\nData ERROR\n\n"; // if there is a problem for some reason go down a line and make a note of it
              sprintf(dataString,"%s,%3.6f,%3.6f,%1.1f,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%u,%u,%u,%u,%u,%u\n",
              constructDateTime(4).c_str(),latitude,longitude,
              gpsSpeed,xAccel,yAccel,
              engRPM,throttlePosition,
              rawEcuMapReading,MAP,turbinePressure,intakeAirTemp,
              rawEcuIatReading,knockValue,ecuTiming,ecuAFR,AirFuelRatio,fuelPressure,
              hybridBatteryCharge,hybridBatteryVoltage,hybridBatteryCurrent,hybridBatteryTemp,
              serialExtractTime,gpsUpdateTime,ioReadTime,dataLogTime,displayUpdateTime,mainLoopTime);
              dataFile.print(dataString);
              //*/
    dataFile.close();
  }
  return(true);
}
void logData()
{
  if(loggingActive) // if logging is on now
  {
    if(newLog) // check if this is a new log
    {
      if(!initializeLog()){
        loggingActive = false; // stop any future logging pocess if the log fails to initialize
        return; // do not continue
      } // make a new log if it fails stop the log
      loggingStatus = logRunning; // note that the logging is now running
      newLog = false; // the log has been made
    }
    dataFile.open(logFileDir, FILE_WRITE);
   ///* Preview of the data writing process
              char dataString [10][280] = {"\nData ERROR\n\n"}; // if there is a problem for some reason go down a line and make a note of it
              sprintf(dataString[0],"%s,%3.6f,%3.6f,", constructDateTime(4).c_str(),latitude,longitude);
              sprintf(dataString[1],"%1.1f,%i,%i,%i,",gpsSpeed,xAccel,yAccel,engRPM);
              sprintf(dataString[2],"%i,%i,%i,", throttlePosition,rawEcuMapReading,MAP);
              sprintf(dataString[3],"%i,%i,%i,",turbinePressure,intakeAirTemp,rawEcuIatReading);
              sprintf(dataString[4],"%3.1f,%3.1f,%3.1f,", knockValue/10.0,ecuTiming/10.0,ecuAFR/10.0);
              sprintf(dataString[5],"%4.2f,%i,%3.1f,", AirFuelRatio/100.0,fuelPressure,hybridBatteryCharge/10.0);
              sprintf(dataString[6],"%i,%3.1f,%i,", hybridBatteryVoltage,hybridBatteryCurrent/10.0,hybridBatteryTemp);
              // serialExtractTime = 98390821;
              // gpsUpdateTime = 91097194;
              // ioReadTime = -96969;
              sprintf(dataString[7],"%u,%u,%u,",serialExtractTime,gpsUpdateTime,ioReadTime);
              // displayUpdateTime = 7287984;
              // mainLoopTime = 248198;
              // dataLogTime = 80088008;
              sprintf(dataString[8],"%u,%u,%u\n", dataLogTime,displayUpdateTime,mainLoopTime);
              // // I think I am missing a %u here which is why main loop time does not log
              // constructDateTime(4).c_str(),latitude,longitude,
              // gpsSpeed,xAccel,yAccel,engRPM,
              // throttlePosition,rawEcuMapReading,MAP,
              // turbinePressure,intakeAirTemp,rawEcuIatReading,
              // knockValue/10.0,ecuTiming/10.0,ecuAFR/10.0,
              // AirFuelRatio/100.0,fuelPressure,hybridBatteryCharge/10.0,
              // hybridBatteryVoltage,hybridBatteryCurrent/10.0,hybridBatteryTemp,
              // serialExtractTime,gpsUpdateTime,ioReadTime,dataLogTime,displayUpdateTime,mainLoopTime);
              for(int i = 0; i < 10; i++)
              {
                dataFile.print(dataString[i]);
              }
              //*/
              
    dataFile.close();
  }
  else
  {
    newLog = true; // start a new log next time
    if(loggingStatus == logRunning) // if the log status was previously running
    loggingStatus = loggingOff; // set the status to off
  }
}

// Idea: have logging functions implemented like screens. There can be a log function pointer that log data calls
// this pointer points to an initialization function that prepares the directory and writes the header text
// then this function changes the pointer to the appropriate log function that writes the values to the file
// to stop this a function is pointed to that does nothing
// then logData() is called the same way as displayScreen()
// also consider passing a pointer to the log variables and writing the filewithin a for loop for cleaner code
// also maybe preparing a string and just writing that would be more time efficent
// although logging does not seem to consume much processor time
char mystr[] = "Time,Lat,Long,Speed,Xg,Yg,RPM,TPS,rawEcuMapReading,MAP,EPress,IAT,rawEcuIatReading,KNK,ecuTiming,ecuAFR,AFR,FuelPress,HybridBatteryCharge,HybridVoltage,HybridCurrent,BatteryTemp,serialExtractTime,gpsUpdateTime,ioReadTime,dataLogTime,displayUpdateTime,loopTime";
char mystre[] = "00:08:50.496,0.000000,0.000000,0.0,0,0,0-13,5,20,0,5,0.0,0.0,0.0,7.42,0,0.0,0,0.0,0,98390821,91097194,4294870327,80088008,7287984,248198";