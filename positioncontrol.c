#include "positioncontrol.h"
#include "utilities.h"
#include <xc.h>
#include "encoder.h"

void position_control_init(){
    // //digital output for testing purpose (RD10)
    // TRISDbits.TRISD10=0;
    // LATDbits.LATD10=1;
    //Timer3 for ISR 200Hz position control
    T3CONbits.TCKPS=0b011;
    PR3=49999;
    TMR3=0;
    T3CONbits.ON=1;
    IPC3bits.T3IP=5; //priority =5
    IFS0bits.T3IF=0;
    IEC0bits.T3IE=1;
}
int pid_control(int p_ref, int p_act){
    int e=0;
    int u=0;
    int de=0;
    e=p_ref-p_act;
    p_eint=(p_eint+e);
    de=(e-prev)/0.05;
    prev=e;

    u=p_ks[0]*e + p_ks[1]* p_eint+ p_ks[2]* de;
    
    if(u>500){
        u=500;
    }else if (u<-500)
    {
        u=-500;
    }
    return u;
}
void clear_traj_arr(){
    int i;
    for(i=0;i<2000;i++){
        traj_array[i]=0;
    }
}