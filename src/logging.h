#include <TimeLib.h>
#include <SdFat.h>

#define SD_FAT_TYPE 3
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#define SD_CONFIG SdioConfig(FIFO_SDIO)
SdFs SD;
FsFile dataFile;

time_t getTeensy3Time();
String millisecond();
void updateMillisecond();
String constructDateTime(uint8_t i);

//void FilePrintTime(FsFile dataFile, char fullTime);