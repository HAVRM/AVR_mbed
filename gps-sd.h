//  Author: hiroki.mine

#ifndef GPS_SD_H_20161224_
#define GPS_SD_H_20161224_
#include "jmap.h"
#include "sd.h"
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

int two_from_six(int data,int keta){ //data(abcdef)->keta:0(ef),..
  if(keta!=0)return data/(100*keta)-(data/(100*(keta+2)))*100;
  else return data-(data/100)*100;
}

int open_sd(int date,int time){ //date:yymmdd,time:hhmmss
  int fd;
  char fname[]="gps.txt";
  if(sd_mount(0) < 0)return -1;
  SD_SET_DATE(2000+two_from_six(date,2),two_from_six(date,1),two_from_six(date,0));
  SD_SET_TIME(two_from_six(time,2),two_from_six(time,1),two_from_six(time,0));
  if ((fd = sd_open(fname, SD_O_CREAT|SD_O_APPEND, 0)) < 0)return -2;
  return fd;
}

void get_gps(int *time,int *date,int *sats,float *east,float *north,float *speed){
  Serial gps;
  int i,temp;
  char c;
  char gga[100], vtg[100], zda[100];
  int n=0,k=0; //n is numbers of message, k is numbers in messege
  int get_id=0;
  while(1){
    c=gps.getc();
    if(c=='$'){
      c=gps.getc();c=gps.getc();c=gps.getc();
      if(c=='G'}{
        c=gps.getc();
        if(c=='G'){
          c=gps.getc();c=gps.getc();
          for(i=0;c!='*';i++){
            c=gps.getc();
            gga[i+1]=c;
          }
          c=gps.getc();c=gps.getc();
          gga[0]=i;
          get_id=get_id|0b001;
        }
      }else if(c=='V'){
        c=gps.getc();c=gps.getc();c=gps.getc();
        for(i=0;c!='*';i++){
          c=gps.getc();
          vtg[i+1]=c;
        }
        c=gps.getc();c=gps.getc();
        vtg[0]=i;
        get_id=get_id|0b010;
      }else if(c=='Z'){
        c=gps.getc();c=gps.getc();c=gps.getc();
        for(i=0;c!='*';i++){
          c=gps.getc();
          zda[i+1]=c;
        }
        c=gps.getc();c=gps.getc();
        zda[0]=i;
        get_id=get_id|0b100;
        if(get_id==0b111)break;
      }
    }
  }

}

int write_sd(int fd,int time,int date,int sats,float east,float north,float speed);
void show_map(float east,float north);
void close_sd(int fd);

#endif
