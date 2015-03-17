#include "em_emu.h"
#include "em_adc.h"

#include "readTemp.h"

#define NRSAMPLES 5

void ADC0_IRQHandler(void) //clears the ADC interuptflag
{
  ADC_IntClear(ADC0, ADC_IF_SINGLE);
}

void TEMP_init()
{
  /* Base the ADC configuration on the default setup. */
  ADC_Init_TypeDef       init  = ADC_INIT_DEFAULT;
  ADC_InitSingle_TypeDef sInit = ADC_INITSINGLE_DEFAULT;

  /* Initialize timebases */
  init.timebase = ADC_TimebaseCalc(0);
  init.prescale = ADC_PrescaleCalc(400000, 0);
  ADC_Init(ADC0, &init);

  /* Set input to temperature sensor. Reference must be 2.5V */
  sInit.reference = adcRef2V5;
  sInit.input     = adcSingleInpCh0; //PD0
  ADC_InitSingle(ADC0, &sInit);

  /* Setup interrupt generation on completed conversion. */
  ADC_IntEnable(ADC0, ADC_IF_SINGLE);
  NVIC_EnableIRQ(ADC0_IRQn);
}

int TEMP_readCelsius(){ // return is 10 times celcius value!
    int result;
    uint32_t temp = 0;
    
    uint32_t i;
    for(i=0; i<NRSAMPLES; i++){
            /* Start one ADC sample */
        ADC_Start(ADC0, adcStartSingle);
        
        while( !(ADC0->STATUS & ADC_STATUS_SINGLEDV)); //wait for data to be valid
        
        temp += ADC_DataSingleGet(ADC0); 
    }

    temp /= NRSAMPLES;
    
    result = (int)(temp)*10;
    result /= 16.4; //ADC measurement to celcius constant
    return result;
}


