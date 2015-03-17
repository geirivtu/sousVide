#ifndef __SSR_H
#define __SSR_H

#define SSR_GPIO_PORT    gpioPortD
#define SSR_GPIO_PIN     1

#include <stdint.h>

void SSR_Init();
void SSR_Set(uint32_t dutycycle);
void SSR_TurnOn();
void SSR_TurnOff(); 
             
#endif //__SSR_H