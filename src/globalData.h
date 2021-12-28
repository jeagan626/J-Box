#ifndef GLOBALDATA_H
#define GLOBALDATA_H
// idea: put all the data shared by all the files here
// this way if some files are not called or included in a particular build there arent any interdependancies
// since these values could be derived many ways
extern int engRPM;
extern bool loggingActive;
extern bool loggingSuccessful;
enum loggingState {logRunning,sdError,dirError,fileError,loggingOff};
extern loggingState loggingStatus;

#endif