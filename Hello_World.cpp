#include "mbed.h"
#include "gps-sd.h"

BusOut col(PC_0,PC_1,PC_2,PC_3,PC_4);
BusOut row(PD_2,PD_3,PD_4,PD_5,PD_6,PD_7,PB_7);
BusIn sw(PB_0,PB_1);
DigitalIn dip(PB_6);
DigitalOut sdcs(PB_2);

int main(void){
  int time=0,date=161226,sats=0;
  float east=0,north=0,speed=0;
  int i=0;
  int k=0;
  int fd;
  char buf[16] = "0123456789\r\n";

  col=0;
  row=~1;
  col=1;
  sdcs=1;
  if((fd=open_sd(date,time))<0)while(1)row=row|(~2);
  col=2;
  while(1){
    col=3;
    get_gps(&time,&date,&sats,&east,&north,&speed);
    col=4;
    if(write_sd(fd,time,date,sats,east,north,speed)<0)while(1)row=row|(~2);
    col=5;
    if(sw&1)break;
    if(sw==0b10)show_map(east,north);
  }
  col=6;
  if(close_sd(fd)<0)while(1)row=row|(~2);
  sdcs=0;
  col=0;
  row=~0;


/*  for(i = 0; i < 10; i++) {
    if (sd_write(fd, buf, 10) <= 0) {
      while(1)row=row|(~8);
    }
  }
  row=~1;
  col=4;
  if (sd_close(fd) < 0) {
    while(1)row=row|(~16);
  }
  row=~1;
  col=5;
  if (sd_unmount() < 0) {
    while(1)row=row|(~32);
  }
  col=0;
  row=~0;




/*  while(1){
    for(int o=0;o<7;o++){
      for(int n=0;n<45;n++){
        dip=(o+n/5)%2;
        for(int m=0;m<100;m++){
          k%=7;
          i=0;
          //for(int l=0;l<5;l++)i=i|(JMP(k+o*7,l+n)<<(4-l));
          for(int l=0;l<5;l++)i=i|(JMP(k,l)<<(4-l));
          col=0;
          row=~(1<<(6-k));
          col=i;
          //col=place_col_5bit(139.580,35.584,k);
          wait_ms(1);
          k++;
        }
      }
    }
  }*/
  return 0;
}
