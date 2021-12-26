#include <TimeLib.h>
#include <SdFat.h>

time_t getTeensy3Time();
String millisecond();
void updateMillisecond();
String constructDateTime(uint8_t i);
void initializeSD();
//void FilePrintTime(FsFile dataFile, char fullTime);