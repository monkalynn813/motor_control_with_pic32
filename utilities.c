#include "utilities.h"                   
#include <xc.h>




void set_mode(enum mode_t m){
    mode=m;
}

enum mode_t get_mode(){
    return mode;

}
void set_current_gains(float p,float i){
    i_ks[0]=p;
    i_ks[1]=i;
}
float* get_current_gains(){
    return i_ks;
}
void set_position_gains(float p,float i, float d){
    p_ks[0]=p;
    p_ks[1]=i;
    p_ks[2]=d;
}
float* get_position_gains(){
    return p_ks;
}