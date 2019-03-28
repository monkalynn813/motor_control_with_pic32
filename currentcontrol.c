#include "currentcontrol.h"
#include "utilities.h"
#include <xc.h>
#include <stdlib.h>

void current_control_init(){
    //RB1 : pin B1 digital I/O
    TRISBbits.TRISB1=0; //RB1 is digital output
    LATBbits.LATB1=0; //initial to high
    //Timer4 for ISR 5KHz current control
    T4CONbits.TCKPS=0;
    PR4=15999;
    TMR4=0;
    T4CONbits.ON=1;
    IPC4bits.T4IP=2; //priority =2
    IFS0bits.T4IF=0;
    IEC0bits.T4IE=1;
    //Timer 2 for 20kHz PWM
    T2CONbits.TCKPS=0;
    PR2=3999;
    TMR2=0;
    OC1CONbits.OCM=0b110;
    OC1CONbits.OCTSEL=0;
    OC1RS=1000;// duty cycle = OC1RS/(PR2+1) = 25%
    OC1R=1000;
    T2CONbits.ON=1;
    OC1CONbits.ON=1;
}
void set_dir(int k){
    if(k<0){
        LATBbits.LATB1=0;
    }else if(k>1){
        LATBbits.LATB1=1;
    }
    
}
void set_pwm(int k){
    pwm= ((PR2+1)*abs(k))/100;
    set_dir(k);

}
int get_pwm(){
    return pwm;
}
int pi_control(int iref, float i_act){
    float e=0;
    float u=0;

    e=iref-i_act;
    i_eint=i_eint+e;
    u=i_ks[0]*e+ i_ks[1]*i_eint; 
    //add limit to u;
    if(u>100.0){
        u=100.0;
    }else if (u<-100.0)
    {
        u=-100.0;
    }
    set_pwm((int)u);
   return pwm;       

}