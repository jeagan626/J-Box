#include "logging.h"

char logFile[13] = "Log"; //Start each logfile with "Log"
char dir[64] = "/Datalogs/"; //Store the log files in a folder called "datalogs"
char logFileDir[77] = " "; // allocate a string to lead to the datalog file we will create
elapsedMillis milliseconds;

time_t getTeensy3Time()
{
  return (Teensy3Clock.get() - (8 * 3600)); // Shift the time so it is consistant with PST
}

String millisecond()
  {
    char milisString [4] = "000";
    sprintf(milisString,"%03i",milliseconds % 1000);
    //char *str = malloc(4); // allocate a location to pass on the string
    //strcpy(str,milisString); // copy the desired string to pass
    //free(str); // free the allocated space
    return (milisString);
  }
String constructDateTime(uint8_t i)
{
  char dateTimeString[16] = "";
  switch (i)
  {
  case 0:
     sprintf(dateTimeString,"%02i.%i",month(),day());
    break;

  case 1:
    sprintf(dateTimeString,"%02i'%02i",hour(),minute());
    break;
  } 
  return dateTimeString;
}

  void initilizeSD(){
      SD.begin(SD_CONFIG);
      strcat(dir,constructDateTime(0).c_str()); //add the current date to the dir to make a dated folder within "datalogs"
      SD.mkdir(dir); // create the dated folder
      strcat(logFile,constructDateTime(1).c_str()); // add the current time to the file name
      strcat(logFile,".csv");// tack on the .txt so we can open it later
      strcpy(logFileDir,dir);//add the current directory to the log file directory string
      strcat(logFileDir,"/");//add the / so we can locate our file within the current directory
      strcat(logFileDir,logFile); // add the log file name to the directory so we can create the file
      dataFile.open(logFileDir, FILE_WRITE);
      dataFile.print("\n\n\n----------------\nBegin New Entry:\n");
      dataFile.print(year());
      dataFile.print('/');
      dataFile.print(month());
      dataFile.print('/');
      dataFile.print(day());
      dataFile.print(' ');
      dataFile.print(hour());
      dataFile.print(':');
      dataFile.print(minute());
      dataFile.print(':');
      dataFile.println(second());
      dataFile.print("\n\n\n");
      dataFile.close();
  }