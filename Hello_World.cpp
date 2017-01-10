#include "mbed.h"

BusOut col(PC_0,PC_1,PC_2,PC_3,PC_4);
BusOut row(PD_2,PD_3,PD_4,PD_5,PD_6,PD_7,PB_7);
BusIn sw(PB_0,PB_1);
DigitalOut dip(PB_6);
DigitalOut sdcs(PB_2);
Serial gps;

#include "gps-sd.h"

int main(void){
  int time[3]={0},date[3]={16,12,26},gll;
  int sats=0;
  float east=135,north=35,high=0,speed=0,dire=0;
  int i=0;
  int k=0;
  long l=0;
  int fd;
  char buf[16] = "0123456789\r\n";
  int re;
  USART_Init(9600);
  if(sw&1)return 0;
  dip=0;
  col=0;
  row=~1;
  col=1;
  sdcs=1;
  char c;
  get_now(date,time);
  col=2;
  for(int j=0;j<1000;j++){
    if((fd=open_sd(date,time,j))<0)while(1)row=fd;
    col=3;
    l=0;
    while(l<1800){
      dip=!dip;
      col=0;
      gll=get_gps(time,date,&sats,&east,&north,&high,&speed,&dire);
      if(sw==0b10)col=5;
      else col=0;
      if((re=write_sd(fd,gll,time,date,sats,east,north,high,speed,dire))<0)while(1)row=re;
      l++;
      if(sw&1)break;
      if(sw==0b10)show_map(east,north);
      else col=0;
      if(sw==0b10)col=3;
    }
    col=6;
    if((re=close_sd(fd))<0)while(1)row=re;
    if(sw&1)break;
  }
  col=7;
  
  sdcs=0;
  col=0;
  row=~0;
  return 0;
}
