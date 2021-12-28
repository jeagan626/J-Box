#include "logging.h"
#include "gps.h"
#include "globalData.h"
char logFile[13] = "Log"; //Start each logfile with "Log"
char dir[64] = "/Datalogs/"; //Store the log files in a folder called "datalogs"
char logFileDir[77] = " "; // allocate a string to lead to the datalog file we will create
bool newLog = true;
#define SD_FAT_TYPE 3
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#define SD_CONFIG SdioConfig(FIFO_SDIO)
SdFs SD;
FsFile dataFile;
//elapsedMillis milliseconds;
//unsigned int lastSeconds = 0;
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
// void updateMillisecond() // this might not be a good idea since the loop speed may be slow enough to constantly reset miliseconds mid second
// {
//   if(second() - lastSeconds > 0) // if one second has passed
//   {
//     lastSeconds = second(); // reset the counter
//     if(milliseconds % 1000 != 0) // check to see if milliseconds has rolled over
//      {
//      // if it hasn't just rolled over then reset it
//       milliseconds = 0;
//     }
//   } 
// }

// 3 
String constructDateTime(uint8_t i)
{ 
  char dateTimeString[20] = "";
  switch (i)
  {
    case 0:
      sprintf(dateTimeString,"%.4i/%.2i/%.2i",year(),month(),day());
      break;

    case 1:
      sprintf(dateTimeString,"%02i'%02i",hour(),minute());
      break;
    
    case 2:
      sprintf(dateTimeString,"%.2i/%.2i/%.2i %.2i:%.2i:%.2i",month(),day(),year()%1000,hour(),minute(),second());
      break;
    
    case 3:
      sprintf(dateTimeString,"%.2i/%.2i/%.2i %.2i:%.2i:%.2i.%.2i",month(),day(),year()%1000,hour(),minute(),second(),rtc_ms() / 10);
      break;

    case 4:
      sprintf(dateTimeString,"%.2i:%.2i:%.2i.%.3i",hour(),minute(),second(),rtc_ms());
      break;

    case 5:
      sprintf(dateTimeString,"%.4i/%.2i/%.2i %.2i:%.2i:%.2i",year(),month(),day(),hour(),minute(),second());
      break;
  }
  return dateTimeString;
}

void initializeSD(){
      SD.begin(SD_CONFIG);
      strcat(dir,constructDateTime(0).c_str()); //add the current date MM-YYYY to the dir to make a dated folder within "datalogs"
      SD.mkdir(dir); // create the dated directory
  }

void makeLog()
{
  strcat(logFile,constructDateTime(1).c_str()); // add the current time to the file name
  strcat(logFile,".csv");// tack on the .txt so we can open it later
  strcpy(logFileDir,dir);//add the current directory to the log file directory string
  strcat(logFileDir,"/");//add the / so we can locate our file within the current directory
  strcat(logFileDir,logFile); // add the log file name to the directory so we can create the file
  dataFile.open(logFileDir, FILE_WRITE);
  dataFile.println(constructDateTime(5)); // print the date and time at the top of the file
  dataFile.print("\n"); // start another line
  dataFile.println("Time,Lat,Long,RPM,Speed,Xacel,Yacel");
  dataFile.close();
}
void logData()
{
  if(loggingActive) // if logging is on now
  {
    if(newLog) // check if this is a new log
    {
      makeLog(); // make a new log
      newLog = false; // the log has been made
    }
    dataFile.open(logFileDir, FILE_WRITE);
    dataFile.print(constructDateTime(4).c_str());
    dataFile.print(',');
    dataFile.print(latitude);
    dataFile.print(',');
    dataFile.print(longitude);
    dataFile.print(',');
    dataFile.print(engRPM);
    dataFile.print(',');
    dataFile.print(gpsSpeed);
    dataFile.print(',');
    dataFile.print(xAccel);
    dataFile.print(',');
    dataFile.print(yAccel);
    dataFile.print('\n');
    dataFile.close();
  }
  else
  {
    newLog = true; // start a new log next time
  }
}