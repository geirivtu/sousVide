#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "segmentlcd.h"

#include "menuAlt.h"
#include "PID.h"
#include "timer.h"

#define MENU_SENSITIVITY 6 //Slider reaction 
#define MAXTIME 3000 //Max time you can set timer to
#define TIMERMULT 10 //Faster scrolling when setting time

static bool ShowTemp = true; //Determens what will be show in STATUSSUB, temp or time

enum state{
    TEMP = 0,
    TIMER,
    STATUS,
    OFF,
    TEMPSUB,
    TIMERSUB,
    STATUSSUB
};

enum state current_state = TEMP;

char *menuItems[]=
{
"SETTEMP",
"TIMER",
"STATUS",
"OFF",
};


struct display{
    int32_t num;
    bool showNum; //If true, display num
    int32_t alphaNum;
    char alpha[8];
    uint32_t circleSeg; //How many of the 8 circle segments to turn on
    bool showCircleSeg; 
};
static struct display Display;

void MENU_Init(){
   Display.alphaNum = 0;
    
  /* Initialize LCD controller without boost */
  SegmentLCD_Init(false);
  strcpy(Display.alpha, menuItems[TEMP]);
  
  PID_Init();
    
    
}

void timerRing(uint32_t on);

//Updates LCD with values set by the events 
void MENU_update(){
    
    char string[8];
    int32_t tmp;
    
    //This check should be done somewhere else..
    if(TIMER_finished()){
        PID_setPoint(0); //Turn off timer, 
    }
    
    switch(current_state){
        case TEMP:       
        case TIMER:
        case STATUS:
        case OFF:
            SegmentLCD_NumberOff();
            timerRing(0);
            SegmentLCD_Write(Display.alpha); 
            break;
        case TEMPSUB:
            SegmentLCD_Write(Display.alpha);
            break;
        case TIMERSUB:
            SegmentLCD_Write(Display.alpha);
            break;
        case STATUSSUB:
            timerRing(1);
            if(ShowTemp){
                SegmentLCD_Number(PID_getSetPoint());
                tmp = PID_getTemp();
                snprintf(string, 8, "%3d,%1d%%C", tmp/10, tmp%10);
            }else{//Show remaining time
                snprintf(string, 8, "%2dt %2dm",TIMER_getRemaining()/60, TIMER_getRemaining()%60);
            }
            SegmentLCD_Write(string);
            break;
        default:
            break;
    }    
}



void event_buttonSelect(){
    
    
    switch(current_state){
        case TEMP: 
            Display.alphaNum = 0;
            snprintf(Display.alpha, 8, "%5d%%C", Display.alphaNum);
            current_state = TEMPSUB;
            break;       
        case TIMER:
            Display.alphaNum = 0;
            snprintf(Display.alpha, 8, " 0t %2dm", Display.alphaNum);
            current_state = TIMERSUB;
            break;
        case STATUS:
            
            current_state = STATUSSUB;
            break;
        case OFF:
            PID_setPoint(0);
            break;
        case TEMPSUB:
            PID_setPoint(Display.alphaNum);
            current_state = STATUSSUB;
            break;
        case TIMERSUB:
            TIMER_setMin(Display.alphaNum);
            current_state = STATUSSUB;
            break;
        case STATUSSUB:
            break;
        default:
            break;
    }
    
    
}



void event_buttonBack(){
    
    switch(current_state){
        case TEMP: 
            break;       
        case TIMER:
            break;
        case STATUS:
            break;
        case OFF:
            break;
        case TEMPSUB:
            strcpy(Display.alpha, menuItems[TEMP]);
            current_state = TEMP;
            break;
        case TIMERSUB:
            strcpy(Display.alpha, menuItems[TIMER]);
            current_state = TIMER;
            break;
        case STATUSSUB:
            strcpy(Display.alpha, menuItems[STATUS]);
            current_state = STATUS;
            break;
        default:
            break;
    }    
    
    
    
}




void event_sliderDiff(int32_t diff){
    
    switch(current_state){
        case TEMP: 
            if(diff >= MENU_SENSITIVITY){
                strcpy(Display.alpha, menuItems[TIMER]);
                current_state = TIMER;
            }
            break;       
        case TIMER:
            if(diff >= MENU_SENSITIVITY){
                strcpy(Display.alpha, menuItems[STATUS]);
                current_state = STATUS;
            }else if(diff <= -MENU_SENSITIVITY){
                strcpy(Display.alpha, menuItems[TEMP]);
                current_state = TEMP;
            }
            break;
        case STATUS:
            if(diff >= MENU_SENSITIVITY){
                strcpy(Display.alpha, menuItems[OFF]);
                current_state = OFF;
            }else if(diff <= -MENU_SENSITIVITY){
                strcpy(Display.alpha, menuItems[TIMER]);
                current_state = TIMER;
            }
            break;
        case OFF:
            if(diff <= -MENU_SENSITIVITY){
                strcpy(Display.alpha, menuItems[STATUS]);
                current_state = STATUS;
            }
            break;
        case TEMPSUB:
            Display.alphaNum += diff;
            if(Display.alphaNum < 0){
                Display.alphaNum=0;
            }else if(Display.alphaNum > 99){
                Display.alphaNum = 99;
            }
            snprintf(Display.alpha, 8, "%5d%%C", Display.alphaNum);
            break;
        case TIMERSUB:
            Display.alphaNum += diff*TIMERMULT;
            if(Display.alphaNum < 0){
                Display.alphaNum=0;
            }else if(Display.alphaNum > MAXTIME){
                Display.alphaNum = MAXTIME;
            }
            snprintf(Display.alpha, 8, "%2dt %2dm",Display.alphaNum/60, Display.alphaNum%60);
            break;
        case STATUSSUB:
            if(diff >= MENU_SENSITIVITY || diff <= -MENU_SENSITIVITY){
                ShowTemp ^= true; //Toggle
            }
            break;
        default:
            break;
    }    
    
    
}


void event_sliderChange(int32_t change){
    int32_t tmp = 0;
    
    switch(current_state){
        case TEMP: 
            break;       
        case TIMER:
            break;
        case STATUS:
            break;
        case OFF:
            break;
        case TEMPSUB:
            if(Display.alphaNum+change < 0){
                tmp=0;
            }else if(Display.alphaNum+change > 99){
                tmp = 99;
            }else{
                tmp = Display.alphaNum+change;
            }
            snprintf(Display.alpha, 8, "%5d%%C", tmp);
            break;
        case TIMERSUB:
            if(Display.alphaNum+change*TIMERMULT < 0){
                tmp=0;
            }else if(Display.alphaNum+change*TIMERMULT> MAXTIME){
                tmp = MAXTIME;
            }else{
                tmp = Display.alphaNum + change*TIMERMULT;  
            }
            snprintf(Display.alpha, 8, "%2dt %2dm",tmp/60, tmp%60);
            break;
        case STATUSSUB:
            break;
        default:
            break;
    }    
    
    
}




void timerRing(uint32_t on){
    
    uint32_t i;
    

    if(on){
        
        uint32_t tar = TIMER_getTarget();
        uint32_t rem = TIMER_getRemaining();
        
        uint32_t nrOfSegToTurnOff = (tar-rem)*100;
        //if tar = 0 then timer isn't turned on
        if(tar != 0){
            nrOfSegToTurnOff = ((nrOfSegToTurnOff/tar)*8)/100;
            nrOfSegToTurnOff = nrOfSegToTurnOff;
        }else{
            nrOfSegToTurnOff = 8;
        }
        
        for(i=0; i<8; i++){
            if(nrOfSegToTurnOff>i){
                SegmentLCD_ARing(i, 0);
            }else{
                SegmentLCD_ARing(i, 1);
            }
        }
    }else{
        for(i=0; i<8; i++){
            SegmentLCD_ARing(i, 0);
        } 
    }
}