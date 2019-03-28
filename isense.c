#include "isense.h"                   
#include <xc.h>
#define SAMPLE_TIME 10 

void adc_init(void){
    AD1PCFGbits.PCFG0=0; //AN0 is adc pin
    AD1CON3bits.ADCS=2;
    AD1CON1bits.ADON=1;
}

unsigned int adc_sample_convert(int pin) { // sample & convert the value on the given 
                                           // adc pin the pin should be configured as an 
                                           // analog input in AD1PCFG
    unsigned int elapsed = 0, finish_time = 0;
    AD1CHSbits.CH0SA = pin;                // connect chosen pin to MUXA for sampling
    AD1CON1bits.SAMP = 1;                  // start sampling
    elapsed = _CP0_GET_COUNT();
    finish_time = elapsed + SAMPLE_TIME;
    while (_CP0_GET_COUNT() < finish_time) { 
      ;                                   // sample for more than 250 ns
    }
    AD1CON1bits.SAMP = 0;                 // stop sampling and start converting
    while (!AD1CON1bits.DONE) {
      ;                                   // wait for the conversion process to finish
    }
    return ADC1BUF0;                      // read the buffer with the result
}

unsigned int get_adc_counts(){
    return adc_sample_convert(0);
}
float get_adc_ma(){
    float am=0;
    am=0.787*get_adc_counts()-397.135;
    return am;
}