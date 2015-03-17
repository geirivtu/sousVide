#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <segmentlcd.h>
#include <stdbool.h>
#include "em_emu.h"
#include "rtc.h"
#include "capsense.h"
#include <string.h>

#include "menu.h"
#include "PID.h"

static bool clearDisplay = false;
static int8_t currentItem = 1;
int sizeMenu = 5;

char *menu[]=
{
"Menu",
"SETTEMP",
"TIMER",
"STATUS",
"HEATOFF",
};

typedef enum
{
  MENU_SLEEP = 0,
  MENU_SENSE = 1,
  MENU_SHOW_ITEM = 2,
} MENU_States_TypeDef;

int targetTemp10 = 0; //Target temp, divide by 10 to get celsius
int targetTime = 0; //Target time
static volatile MENU_States_TypeDef menuState = MENU_SENSE;
volatile int activesubMenu=0;
static int touch = 0;   // flytta den hit for jeg må vite status

int iTempPredict(int iTemporaryDiff);
int iTempPredict(int iTemporaryDiff);
int getCapSenseScrollInterval(void);
//void capSenseChTrigger(void); void capSenseScanComplete(void);


void Menu_Init(){
  CAPSENSE_Init();
  CAPSENSE_setupLESENSE(false);
  /* Setup capSense callbacks. If not included it will trigg a bug in driver*/
  //CAPSENSE_setupCallbacks(&capSenseScanComplete, &capSenseChTrigger);
}

void subMenu(void);

void Menu_Update(void){
  
  if(activesubMenu>0){
    subMenu();
  }else{
    switch(menuState){
      case MENU_SENSE:{      
        getCapSenseScrollInterval();
        break;
      }
     case MENU_SHOW_ITEM:{
         SegmentLCD_Write(menu[currentItem]);
         menuState=MENU_SENSE;
         break;
       }
     default:{
        break;
      }
     }
  }
 }
  
void Menu_Back(void){
  if(activesubMenu!=0){
    activesubMenu=0;
    setClearDisplay(true);
  }
  menuState=MENU_SHOW_ITEM;
}

void Menu_Select(void){
  if(activesubMenu==0){
    switch (currentItem){
    // "Menu", 
    case 0: {
      break;
    }
    // "Set temp",
    case 1:{
      activesubMenu=1;
      break;
    }
    
    //"Set time",
    case 2: {
      activesubMenu=2;
      break;
    }
    //"Show status",
    case 3: {
      activesubMenu=3;
      break;
    }
    //"Heat off",
    case 4: {
      //Call PID with target 0 to turn off heater
      PID_setPoint(0);
      targetTemp10 = 0;
      break;
    }
   default:{
     break;
    }
    }
  }else if(activesubMenu==1){  
    //Do action if in submenu 1, set temp   
    //Call PID with targetTemp10/10 
    PID_setPoint(targetTemp10);
    activesubMenu = 3; //Switch to show status
  }
  else if(activesubMenu==2)
  { 
    TIMER_setMin(targetTime);
    //Do action if in submenu 2, set timer
  }
  
}

void subMenu(void){
  if(activesubMenu>0){
      switch (activesubMenu){
        // do nothing
        case 0:{
            break;
          }
        // set temp
        case 1:{
            
            int scrollValue = getCapSenseScrollInterval();
            targetTemp10 += scrollValue*3;
            if (targetTemp10 > 999)
            {
              targetTemp10=999;
            }
            else if(targetTemp10 < 0)
            {
              targetTemp10=0;
            }
            if(touch!=1) //hack kjører så vil ikke se dette kjøre da
            {
            char string[8];
            snprintf(string, 8, " %2d%%C", (targetTemp10 / 10), targetTemp10 % 10);
            SegmentLCD_Write(string); 
            }
            break;
          }
          case 2:{
            int scrollValue = getCapSenseScrollInterval();
            targetTime += scrollValue*3;
            if (targetTime > 999)
            {
              targetTime=999;
            }
            else if(targetTime < 0)
            {
              targetTime=0;
            }
            if(touch!=1) //hack kjører så vil ikke se dette kjøre da
            {
            char string[8];
            snprintf(string, 8, " %2d m", targetTime);
            SegmentLCD_Write(string); 
            }
            break;
          }       
        case 3:{
            //Get target from PID, write to segmentLCD_Number(diff);
            int targetTemp = PID_getSetPoint(); //Fetch from PID
            //SegmentLCD_NumberHack(targetTemp/10);
            SegmentLCD_Number(targetTemp/10);
            SegmentLCD_Symbol(LCD_SYMBOL_DEGC, 1);
            
            //SegmentLCD_Symbol(LCD_SYMBOL_DP10, 1);
            
            //Get actual temp from PID and display with  SegmentLCD_Write(temp);
            int measuredTemp = PID_getTemp(); //Fetch from PID
            char string[8];
            //SegmentLCD_Symbol(LCD_SYMBOL_DP10, 1);
            //SegmentLCD_Symbol(LCD_SYMBOL_DEGC, 0);
            snprintf(string, 8, " %2d%%C", (measuredTemp / 10), measuredTemp % 10);
            SegmentLCD_Write(string); 
            break;
          }
        default:{
            break;
       }
    }
  }
}


int getCapSenseScrollInterval(void){
  static int oldSliderPos=-1;
  static int oldSliderPos2=-1;

  int diff = 0;
  int slider = CAPSENSE_getSliderPosition();
  
  if (slider != -1 && touch==0){
    oldSliderPos = slider;
    touch = 1;
  }
  if (slider != -1 && touch==1){
    //SegmentLCD_Number(oldSliderPos2-oldSliderPos); //Debug
    oldSliderPos2 = slider;
    // hack to predict temp during scroll
    if(activesubMenu==1)
    {
      int disp = iTempPredict(oldSliderPos2-oldSliderPos);
      char string[8];
      snprintf(string, 8, " %2d%%C", (disp / 10), disp % 10);
      SegmentLCD_Write(string); 
    }
  }
   if(activesubMenu==2)
    {
      int disp = iTimePredict(oldSliderPos2-oldSliderPos);
      char string[8];
      snprintf(string, 8, " %2d m", targetTime);
      SegmentLCD_Write(string); 
    }
  if (slider == -1 && touch==1){
    diff = oldSliderPos2-oldSliderPos;
    //SegmentLCD_Number(diff); //debug
    touch = 0;
    //If not in a submenu
    if(activesubMenu==0){
      if(diff > 15){
        if(currentItem+1 < sizeMenu){
          currentItem++;
        }
      }else if(diff < -15){  
        if(currentItem > 1){
          currentItem--;
        }
      }
    }
  }  
  menuState=MENU_SHOW_ITEM;
  return diff;
 }


/**************************************************************************//**
 * @brief  Callback for sensor scan complete.
 *****************************************************************************/
//void capSenseScanComplete(void){
  
//}

/**************************************************************************//**
 * @brief  Callback for sensor channel triggered.
 *****************************************************************************/
//void capSenseChTrigger(void){
//}

int iTempPredict(int iTemporaryDiff)
{
  int target = targetTemp10 + iTemporaryDiff*2;
  if (target > 999)
  {
    target=999;
  }else if(target < 0)
  {
    target=0;
  }
  return target;
}

int iTimePredict(int iTemporaryDiff)
{
  int target = targetTime + iTemporaryDiff*2;
  if (target > 999)
  {
    target=999;
  }else if(target < 0)
  {
    target=0;
  }
  return target;
}


void setClearDisplay(bool imp)
{
  clearDisplay=imp; 
}

void ClearDisplay(void)
{
  if(clearDisplay)
  {

    SegmentLCD_AllOff();
    setClearDisplay(false);
  }
  
}