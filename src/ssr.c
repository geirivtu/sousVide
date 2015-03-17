#include "efm32.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_timer.h"

#include <stdint.h>

#include "ssr.h"

//14MHz/2/1024 = 6836 -> 4 sec = 27343 = TOP value
#define TOP 27343

void SSR_Init(){
  
  /* Enable clock for GPIO module */
  CMU_ClockEnable(cmuClock_GPIO, true);
  
  /* Enable clock for TIMER0 module */
  CMU_ClockEnable(cmuClock_TIMER0, true);
   
  //Enable HFPERCLOCK  and set prescaler= 2
  //CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockDivSet(cmuClock_HFPER, cmuClkDiv_2);
  
  /* Set CC0 location 3 pin (PD1) as output */
  GPIO_PinModeSet(SSR_GPIO_PORT, SSR_GPIO_PIN, gpioModePushPull, 0);
  
  
  /* Select CC channel parameters */
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
  timerCCInit.mode = timerCCModePWM;
  
  /* Configure CC channel 0 */
  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  /* Route CC0 to location 3 (PD1) and enable pin */  
  TIMER0->ROUTE |= (TIMER_ROUTE_CC0PEN | TIMER_ROUTE_LOCATION_LOC3); 
  
  
  TIMER_TopSet (TIMER0, TOP);
  
  /* Set compare value starting at 0 - it will be incremented in the interrupt handler */
  TIMER_CompareBufSet(TIMER0, 0, 0);

  /* Select timer parameters */  
  TIMER_Init_TypeDef timerInit =
  {
    .enable     = true,
    .debugRun   = true,
    .prescale   = timerPrescale1024,
    .clkSel     = timerClkSelHFPerClk,
    .fallAction = timerInputActionNone,
    .riseAction = timerInputActionNone,
    .mode       = timerModeUp,
    .dmaClrAct  = false,
    .quadModeX4 = false,
    .oneShot    = false,
    .sync       = false,
  };
  
  /* Configure timer */
  TIMER_Init(TIMER0, &timerInit);
   
}

//Power from 0 to 100
void SSR_Set(uint32_t dutycycle){
  
  uint32_t value;
  
  if (dutycycle<5){
    value = 0;
  }
  else if(dutycycle>95){
    value = TOP+1;
  }
  else{
    value = (TOP/100)*dutycycle;
  }
  
  TIMER_CompareBufSet(TIMER0, 0, value); 
  
}

void SSR_TurnOff(){
  //GPIO_PinOutClear(SSR_GPIO_PORT, SSR_GPIO_PIN);
  /* Select CC channel parameters */
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
  timerCCInit.mode = timerCCModeOff;
  
  /* Configure CC channel 0 */
  TIMER_InitCC(TIMER0, 0, &timerCCInit);
}

void SSR_TurnOn(){
  //GPIO_PinOutClear(SSR_GPIO_PORT, SSR_GPIO_PIN);
  /* Select CC channel parameters */
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
  timerCCInit.mode = timerCCModePWM;
  
  /* Configure CC channel 0 */
  TIMER_InitCC(TIMER0, 0, &timerCCInit);
}

