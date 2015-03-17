#ifndef __PID_H
#define __PID_H

int getNewValue_PID(int setPoint);
#define GOOD_ENOUGH 15
#define SMOOTHING_FACTOR 2.0
#define MEMORY_DEPTH 5



void PID_Init();
int PID_getTemp();
void PID_setPoint(int value);
int PID_getSetPoint();
int PID_runController();



#endif