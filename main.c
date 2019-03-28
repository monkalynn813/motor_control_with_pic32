#include "NU32.h"          // config bits, constants, funcs for startup and UART
#include "encoder.h"// include other header files here
#include "utilities.h"
#include "isense.h"
#include "currentcontrol.h"
#include "positioncontrol.h"
#define BUF_SIZE 200
#define PLOTPTS 100
static volatile int ActPos[2000];
static volatile int ADCarray[PLOTPTS];
static volatile int REFarray[PLOTPTS];
static volatile int i_ready_to_store;
static volatile int p_ready_to_store;
static int traj_size, traj_pos,i,goal_degree;


void __ISR(_TIMER_4_VECTOR, IPL2SOFT) CurrentControl(void) {
  switch (get_mode())
  {
    case IDLE:{
      OC1RS=0; 
      
      break;
    }
    case PWM:{
      OC1RS=get_pwm();
      break;
    }

    case ITEST:{
      static volatile int itestcount;
     
      iread=get_adc_ma();
      ADCarray[itestcount]=(int)iread;
      
      if(itestcount<100){
        if(itestcount==25 || itestcount==50 || itestcount==75){
          itestref=(-1)*itestref;
        }
        REFarray[itestcount]=itestref;
        OC1RS=pi_control(itestref,iread); //update pwm and direction from pi controller
        itestcount++;
      }else
      {
        set_mode(IDLE);
        i_eint=0; //clear integral term after complete control of this time
        itestcount=0;
        i_ready_to_store=1;
      }
      break;
    }
    case HOLD:{
      iread=get_adc_ma();
      OC1RS=pi_control(i_pref,iread);
      break;
    }
    case TRACK:{
      iread=get_adc_ma();
      OC1RS=pi_control(i_pref,iread);
      break;
    }
  
    default:
      
      break;
  }


  IFS0bits.T4IF=0;
}

void __ISR(_TIMER_3_VECTOR, IPL5SOFT) PositionControl(void) {
    static volatile int track_count=0;
switch (get_mode())
{
  case HOLD:{
    pread=encoder_degree();
    
    i_pref=pid_control(goal_degree,pread);
    break;
  }
  case TRACK:{

    if (track_count<traj_size){
      pread=encoder_degree();
      goal_degree=(int)traj_array[track_count];
      ActPos[track_count]=pread;
      i_pref=pid_control(goal_degree,pread);
      track_count++;      
    }else
    {
      goal_degree=traj_array[traj_size-1];
      track_count=0;
      set_mode(HOLD);
      i_eint=0;
      p_eint=0;
      prev=0;
      p_ready_to_store=1;
    }
    

    break;
}
  default:
    break;
}

IFS0bits.T3IF=0;
}
int main() 
{ 
  char buffer[BUF_SIZE];
  NU32_Startup(); // cache on, min flash wait, interrupts on, LED/button init, UART init
  NU32_LED1 = 1;  // turn off the LEDs
  NU32_LED2 = 1;
  set_mode(0); //set to IDLE mode        
  __builtin_disable_interrupts();
  encoder_init();
  adc_init();
  current_control_init();
  position_control_init();
  __builtin_enable_interrupts();

i_ks[0]=2;
i_ks[1]=0.07;
p_ks[0]=60;
p_ks[1]=0;
p_ks[2]=190;
  while(1)
  {
    NU32_ReadUART3(buffer,BUF_SIZE); // we expect the next character to be a menu command
    NU32_LED2 = 1;                   // clear the error LED
    switch (buffer[0]) {
      case 'a':
      {
        sprintf(buffer,"%d \r\n", get_adc_counts());
        NU32_WriteUART3(buffer); // send encoder count to client
        break;
      }
      case 'b':
      {
        sprintf(buffer,"%f \r\n", get_adc_ma());
        NU32_WriteUART3(buffer); // send encoder count to client
        break;
      }
      case 'c':
      {
        sprintf(buffer,"%d \r\n", encoder_counts(1));
        NU32_WriteUART3(buffer); // send encoder count to client
        break;
      }
      case 'd':
      {
        sprintf(buffer,"%d \r\n", encoder_degree());
        NU32_WriteUART3(buffer); // send encoder count to client
        break;
      }
      case 'e':
      {
        encoder_counts(0);
        sprintf(buffer,"%d \r\n", encoder_degree());
        NU32_WriteUART3(buffer); // send encoder count to client
        break;
      }
      case 'f':
      {
        int n;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer,"%d",&n);
        set_pwm(n);
        set_mode(PWM); 
        break;
      }
      case 'g':
      {
        float i_p,i_i;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer,"%f",&i_p);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer,"%f",&i_i);
        set_current_gains(i_p,i_i);
        break;
      }
      case 'h':
      { float *i_gains=get_current_gains();
        sprintf(buffer,"%f %f \r\n", i_gains[0],i_gains[1]);
        NU32_WriteUART3(buffer); // send encoder count to client
        break;
      }
      case 'i': //set position gains
      {
        float p_p,p_i,p_d;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer,"%f",&p_p);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer,"%f",&p_i);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer,"%f",&p_d);
        set_position_gains(p_p,p_i,p_d);
        break;
      }
      case 'j':
      { float *p_gains=get_position_gains();
        sprintf(buffer,"%f %f %f \r\n", p_gains[0],p_gains[1],p_gains[2]);
        NU32_WriteUART3(buffer); // send encoder count to client
        break;
      }
      case 'k': //test current gains
      { i_eint=0;
        itestref=200;
        i_ready_to_store=0;
        set_mode(ITEST);
        while(i_ready_to_store!=1){;}  //wait till array is ready to plot 
        
        sprintf(buffer,"%d \r\n",PLOTPTS);
        NU32_WriteUART3(buffer);
        for(i=0;i<PLOTPTS;i++){
          sprintf(buffer,"%d %d \r\n",REFarray[i],ADCarray[i]);
          NU32_WriteUART3(buffer);
        }
        break;
      }
      case 'l':
      {
        p_eint=0;
        prev=0;
        i_eint=0;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer,"%d",&goal_degree);
        set_mode(HOLD); 
        break;
      }
      case 'm': //load step trajectory
      { int j;
        clear_traj_arr();
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer,"%d",&traj_size);
        for (j=0;j<traj_size;j++){
          NU32_ReadUART3(buffer,BUF_SIZE);
          sscanf(buffer,"%f",&traj_array[j]);
        }
        sprintf(buffer,"%d \r\n",traj_size);
        NU32_WriteUART3(buffer);
        break;
      } 
      case 'n': //load cubic trajectory
      { int k;
        clear_traj_arr();
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer,"%d",&traj_size);
        for (k=0;k<traj_size;k++){
          NU32_ReadUART3(buffer,BUF_SIZE);
          sscanf(buffer,"%f",&traj_array[k]);
        }
        sprintf(buffer,"%d \r\n",traj_size);
        NU32_WriteUART3(buffer);
        break;
      } 
      case 'o':
      { p_eint=0;
        prev=0;
        i_eint=0;
        p_ready_to_store=0;
        set_mode(TRACK);
        while(p_ready_to_store!=1){;}  //wait till array is ready to plot 
        
        sprintf(buffer,"%d \r\n",traj_size);
        NU32_WriteUART3(buffer);
        for(i=0;i<traj_size;i++){
          sprintf(buffer,"%f %d \r\n",traj_array[i],ActPos[i]);
          NU32_WriteUART3(buffer);
        }

 
        break;
      }     
      case 'p':
      {
        set_mode(IDLE); 
        break;
      }
      case 'q':
      { 
        set_mode(0);
        break;
      }
      case 'r':
      {
        sprintf(buffer,"%d \r\n", get_mode());
        NU32_WriteUART3(buffer); 
        break;
      }
      default:
      {
        NU32_LED2 = 0;  // turn on LED2 to indicate an error
        break;
      }
    }
  }
  return 0;
}
