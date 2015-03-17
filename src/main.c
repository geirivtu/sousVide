#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "efm32.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_gpio.h"
//#include "em_adc.h"
#include "em_cmu.h"
//#include "em_rtc.h"
//#include "segmentlcd.h"
//#include "rtc.h"
#include "trace.h"

#include "PID.h"
#include "readTemp.h"
#include "led.h"
#include "timer.h"


#include "slider.h"
#include "menuAlt.h"

void gpioSetup(void);


uint32_t msTicks = 0; 

//Statusflag for scheduler
bool flag_LED = false;
bool flag_PID = false;
bool flag_MenuUpdate = false;
bool flag_TimerTick = false;

//Periods for scheduler

#define MENU_PER 50
#define LEDTOGGLE_PER 1000
#define TIMERTICK_PER 3000
#define PID_PER 2000

/**************************************************************************//**
 * @brief GPIO Interrupt handler (PB1)
 *****************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
   //Menu_Select(); //Event
  event_buttonSelect();
  
  /* Acknowledge interrupt */
  GPIO_IntClear(1 << 11);
}

/**************************************************************************//**
 * @brief GPIO Interrupt handler (PB0)
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void){
    
  //Menu_Back();  //Event
  event_buttonBack();
  
   /* Acknowledge interrupt */
  GPIO_IntClear(1 << 8);
}

/**************************************************************************//**
 * @ 1 ms system tick handler
 *****************************************************************************/
void SysTick_Handler(void){
  msTicks++;       /* increment counter necessary in Delay()*/
  
  if(msTicks%LEDTOGGLE_PER == 0){
    flag_LED = 1;
  }
  if(msTicks%MENU_PER == 0){
    flag_MenuUpdate = 1;
  }
  if(msTicks%TIMERTICK_PER == 0){
    flag_TimerTick = 1;
  }
  if(msTicks%PID_PER == 0){
    flag_PID = 1;
  }
}


/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  /* Chip errata */
  CHIP_Init();

  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_ADC0, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
  
  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;

  /* Initialize LCD controller without boost */
  //SegmentLCD_Init(false); Done in menuAlt

  /* Enable board control interrupts */
  gpioSetup();

  //Setup LED
  LED_Init();
  
  //Setup PID
  PID_Init();
  
  /* Initialize MENU */
  MENU_Init(); //new version
  
  //Setup touch slider
  SLIDER_Init();
  

  //Main loop, chech for button/slider events and status flags. 
  int32_t tmp,tmp2;
  while (1)
  {
      tmp = SLIDER_posDiff();
      if (tmp != 0){
          event_sliderDiff(tmp);
      }
      
      tmp2 = SLIDER_posChange();
      if (tmp2 != 0){
          event_sliderChange(tmp2);
      }

      //Mini scheduler
      if(flag_LED){
        LED_Toggle();
        flag_LED = false;
      }
      if(flag_PID){
        PID_runController();       
        flag_PID = false;
      }
      if(flag_MenuUpdate){
        MENU_update();
        flag_MenuUpdate = false;
      }  
      if(flag_TimerTick){
        TIMER_tick(TIMERTICK_PER);
        flag_TimerTick = false;
      }     
  }
}



/**************************************************************************//**
 * @brief Setup GPIO interrupt for button push
 *****************************************************************************/
void gpioSetup(void)
{
  /* Enable GPIO in CMU */
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure PD8 and PB11 as input */
  GPIO_PinModeSet(gpioPortD, 8, gpioModeInput, 0);
  GPIO_PinModeSet(gpioPortB, 11, gpioModeInput, 0);

  /* Set falling edge interrupt for both ports */
  GPIO_IntConfig(gpioPortD, 8, false, true, true);
  GPIO_IntConfig(gpioPortB, 11, false, true, true);

  /* Enable interrupt in core for even and odd gpio interrupts */
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
}