#include <stdint.h>

#include "PID.h"
#include "readTemp.h"
#include "ssr.h"


static int setPoint;
static int lastMeasuredTemp;

void PID_Init(){
  SSR_Init();
  /* Setup ADC for sampling temperature sensor. */
  TEMP_init();
}


void PID_setPoint(int value){
  setPoint = value*10;
  
}

//setPoint
int PID_getSetPoint(){
  return setPoint/10;
}

//returns 10 times the temp
int PID_getTemp(){
  return lastMeasuredTemp;
}

static float prevValues[] = {0,0,0,0,0};
int PID_runController(){
  static float diff;
  static float accumulator;
  static float propFactor;
  

  
  
  float maxAccu = 20*CELCIUS_FACTOR;
  float Kp = 3;
  float Ki = 0.01;
  float Km = 0.5;
  float output;
  float measurement;
  float good_enough = GOOD_ENOUGH*CELCIUS_FACTOR;
  float memorySum = 0;
  
  measurement = (float)(TEMP_readCelsius()) ;// 10 times actual value
  lastMeasuredTemp = (int)measurement;
  //setPoint *= CELCIUS_FACTOR;
     
  diff = setPoint - measurement;
   
  if (diff < 0){
    accumulator = Ki*diff; //Should perhaps be set to zero, TEST
    propFactor = 0;
  }
  else if (diff < good_enough){
     accumulator += Ki*diff;
     propFactor = Kp*diff;
  }
  else {
    //accumulator += Ki*diff; 
    propFactor = Kp*diff;
  }
  
  if (accumulator > maxAccu){
    accumulator = maxAccu; 
  }
  
  for (int j = MEMORY_DEPTH-1; j>=0;j--){
   
    if (j != 0){
      prevValues[j] = prevValues[j-1];
    }
    else if (j == 0){
       prevValues[0] = diff*Km; 
      
    }
  }
  
  memorySum = 0.0;
    for (int j = 0; j<MEMORY_DEPTH;j++){
    memorySum += prevValues[j];
  }
  
  output = (propFactor + accumulator + memorySum)/CELCIUS_FACTOR;
  if (output > 100){
    output = 100; 
  }
  

  
  SSR_Set((uint32_t)(output));
  
  return (int)(output);
}