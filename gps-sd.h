//  Author: hiroki.mine

#ifndef GPS_SD_H_20161224_
#define GPS_SD_H_20161224_
#include "jmap.h"
#include "sd.h"
#include "gps.h"
#include "serial.h"
#include "mbed.h"

int place_col_5bit(double east,double north,int row){
  double diffe=(east-122.933)/0.527;
  double diffn=(45.538-north)/0.439;
  int e=(int)diffe-2;
  int n=(int)diffn-3+row;
  int ans;
  ans=0;
  for(int i=0;i<5;i++){
    if((e+i)>=50 || (e+i)<0 || n>=50 || n<0);
    else ans=ans|(JMP(n,e+i)<<(4-i));
  }
  return ans;
}

char uint_2_char60(int data){
  data%=60;
  if(data<0)return '@';
  if(data<10)return '0'+data;
  else if(data<36)return 'a'+data-10;
  else return 'A'+data-36;
}

int open_sd(int date[3],int time[3],int j){ //date:yy,mm,dd time:hh,mm,ss number
  int fd;
  char fname[]="dhmm0.txt";
  char buf[13];
  buf[12]='\n';
  fname[0]=uint_2_char60(date[2]-1);
  fname[1]=uint_2_char60(time[0]);
  fname[2]=uint_2_char60(time[1]/30);
  fname[3]=uint_2_char60(time[1]%30);
  fname[4]=uint_2_char60(j);
  if(sd_mount(0) < 0)return -1;
  SD_SET_DATE((2000+date[0]),date[1],date[2]);
  SD_SET_TIME(time[0],time[1],time[2]);
  if ((fd = sd_open(fname, SD_O_CREAT|SD_O_APPEND, 0)) < 0)return -2;
  buf[0]='\r';buf[1]='\n';
  sd_write(fd, buf, 2);
  for(int i=0;i<3;i++){
    buf[i*2]=date[i]/10+48;
    buf[i*2+1]=date[i]%10+48;
  }
  for(int i=0;i<3;i++){
    buf[i*2+6]=time[i]/10+48;
    buf[i*2+7]=time[i]%10+48;
  }
  sd_write(fd, buf, 13);
  return fd;
}

int write_sd(int fd,int gll,int time[3],int date[3],int sats,float east,float north,float high,float speed,float dire){
  //28byte
  char buf[28];
  int temp=0;
  if(gll==-1)buf[0]='N';
  else buf[0]='A';
  buf[1]=uint_2_char60(date[2]);
  for(int i=0;i<3;i++)buf[i+2]=uint_2_char60(time[i]);
  buf[5]=uint_2_char60(sats);
  long tle8=(long)((east-120)*1000000);//:0-30000000
  long tln8=(long)((north-20)*1000000);//:0-30000000
  long tlh6=(long)(high*100);//0-400000
  long tls6=(long)(speed*100);//:0-100000
  long tld5=(long)(dire*100);//:0-36000
  for(int i=0;i<5;i++){
    buf[i+6]=uint_2_char60(tle8%60);
    tle8/=60;
  }
  for(int i=0;i<5;i++){
    buf[i+11]=uint_2_char60(tln8%60);
    tln8/=60;
  }
  for(int i=0;i<4;i++){
    buf[i+16]=uint_2_char60(tlh6%60);
    tlh6/=60;
  for(int i=0;i<3;i++){
    buf[i+20]=uint_2_char60(tls6%60);
    tls6/=60;
  }
  for(int i=0;i<3;i++){
    buf[i+23]=uint_2_char60(tld5%60);
    tld5/=60;
  }
  
  buf[26]='\r';
  buf[27]='\n';
  return sd_write(fd, buf, 28);
}

void show_map(float east,float north){
  for(int i=0;i<10;i++){
    for(int j=0;j<7;j++){
      int k=place_col_5bit(east,north,j);
      col=0;
      row=~(1<<(6-k));
      col=k;
      wait_ms(1);
    }
  }
}

int close_sd(int fd){
  if (sd_close(fd) < 0)return -1;
  if (sd_unmount() < 0)return -2;
  return 0;
}

#endif
