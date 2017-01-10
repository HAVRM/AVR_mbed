#include "mbed.h"

DigitalOut sdcs(PB_2);
DigitalOut led(PB_0);

#include "sd.h"

int main(void){
  int fd;
  char buf[16] = "0123456789\r\n";
  char name[11] = "test.txt";
  sdcs=1;
  if(sd_mount(0)<0){
    while(1){
      led=!led;
      wait_ms(50);
    }
  }
  SD_SET_DATE(2016,12,26);
  SD_SET_TIME(12,30,50);
  if((fd=sd_open(name, SD_O_CREAT|SD_O_APPEND, 0))<0){
    while(1){
      led=!led;
      wait_ms(50+led*50);
    }
  }
  for(int i=0;i<1000;i++){
    if(sd_write(fd, buf, 12)<=0){
      while(1){
        led=!led;
        wait_ms(100);
      }
    }
  }
  if(sd_close(fd)){
    while(1){
      led=!led;
      wait_ms(100+led*100);
    }
  }
  if(sd_unmount()){
    while(1){
      led=!led;
      wait_ms(200);
    }
  }
  sdcs=0;
  return 0;
}
