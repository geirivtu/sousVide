
#include <stdint.h>


static uint32_t targetMs = 0;
static uint32_t elapsedTimeMs = 0;
static uint32_t timerStatus = 0; //0 off, 1 on

void TIMER_setMin(int time){
  elapsedTimeMs = 0;
  timerStatus = 1;
  targetMs = time*1000*60;
}

uint32_t TIMER_finished(){
  
  if (elapsedTimeMs > targetMs && timerStatus==1){
    timerStatus = 0;
    return 1;
  }else{
   return 0; 
  }
}

//returns remaining time
uint32_t TIMER_getRemaining(){
    
    
    if(elapsedTimeMs >= targetMs){
        return 0;
    }else{
        return (targetMs-elapsedTimeMs)/(1000*60);
    }
}

uint32_t TIMER_getTarget(){
    return targetMs/(1000*60);
}

void TIMER_tick(int ms){
    if(elapsedTimeMs <= targetMs){
        elapsedTimeMs += ms;
    }
}