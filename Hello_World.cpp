#include "mbed.h"

Serial gps;
DigitalOut led(PB_0);

#include "gps.h"

int main(void){
  int time[3],date[3],sats;
  float east,north,speed,high,dire;
  while(1){
    if(get_gps(time,date,&sats,&east,&north,&speed,&high,&dire)==1)led=!led;
  }
}
