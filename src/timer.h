
#ifndef __TIMER_H
#define __TIMER_H



void TIMER_setMin(int time);
uint32_t TIMER_getRemaining(void);
uint32_t TIMER_getTarget(void);
void TIMER_tick(int ms);
uint32_t TIMER_finished();

#endif //__TIMER_H