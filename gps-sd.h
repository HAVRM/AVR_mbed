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

int ten_x(int a){ //10^a
  int ans=1;
  for(int i=0;i<a;i++)ans*=10;
  return ans;
}

char uint_2_char60(int data){
  data%=60;
  if(data<10)return 48+data;
  else if(data<30)return 'a'+data-10;
  else return 'A'+data-30;
}

char uint_2_char60_2(int data){
  data%=60;
  if(data<10)return 48+data;
  else if(data<30)return 'a'+data-10;
  else return '_';
}

int open_sd(long date,long time,int j){ //date:yymmdd,time:hhmmss,number
  int fd;
  char fname[]="dhm0.txt";
  fname[0]=uint_2_char60(date-(date/100)*100);
  fname[1]=uint_2_char60(time/10000);
  fname[2]=uint_2_char60(time/100-(time/10000)*100);
  fname[3]=uint_2_char60(j);
  if(sd_mount(0) < 0)return -1;
  int y=2000+date/10000,m=date/100-(date/10000)*100,d=date-(date/100)*100,h=time/10000,p=time/100-(time/10000)*100,s=time-(time/100)*100;
  SD_SET_DATE(y,m,d);
  SD_SET_TIME(h,p,s);
  if ((fd = sd_open(fname, SD_O_CREAT|SD_O_APPEND, 0)) < 0)return -2;
  return fd;
}

int get_gps(long *time,long *date,int *sats,float *east,float *north,float *speed,float *dire){ //get->return 0
  Serial gps;
  int i,temp;
  char c;
  char gga[100], vtg[100], zda[100], ggl;
  int n=0,k=0; //n is numbers of message, k is numbers in messege
  int get_id=0;
  while(1){
    c=gps.getc();
    if(c=='$'){
      c=gps.getc();c=gps.getc();c=gps.getc();
      if(c=='G'){
        c=gps.getc();
        if(c=='G'){
          c=gps.getc();c=gps.getc();c=gps.getc();
          for(i=0;c!='*';i++){
            gga[i+1]=c;
            c=gps.getc();
          }
          c=gps.getc();c=gps.getc();
          gga[0]=i;
          get_id=get_id|0b001;
        }
        else if(c=='L'){
          for(int i=0;c!='*';i++){
            ggl=c;
            c=gps.getc();
          }
        }
      }else if(c=='V'){
        c=gps.getc();c=gps.getc();c=gps.getc();c=gps.getc();
        for(i=0;c!='*';i++){
          vtg[i+1]=c;
          c=gps.getc();
        }
        c=gps.getc();c=gps.getc();
        vtg[0]=i;
        get_id=get_id|0b010;
      }else if(c=='Z'){
        c=gps.getc();c=gps.getc();c=gps.getc();c=gps.getc();
        for(i=0;c!='*';i++){
          zda[i+1]=c;
          c=gps.getc();
        }
        c=gps.getc();c=gps.getc();
        zda[0]=i;
        get_id=get_id|0b100;
        if(get_id==0b111)break;
      }
    }
  }
  if(ggl=='V')return -1;
  *time=0;
  for(int i=0;i<6;i++)*time+=(zda[i+1]-48)*ten_x(5-i);
  *date=0;
  for(int i=0;i<2;i++)*date+=(zda[i+14]-48)*ten_x(3-i);
  for(int i=0;i<2;i++)*date+=(zda[i+17]-48)*ten_x(1-i);
  for(int i=0;i<2;i++)*date+=(zda[i+22]-48)*ten_x(5-i);
  *sats=0;
  for(int i=0;i<2;i++)*sats+=(gga[i+35]-48)*ten_x(1-i);
  *east=0;
  for(int i=0;i<5;i++)*east+=(gga[i+21]-48)*ten_x(4-i);
  for(int i=0;i<2;i++)*east+=(gga[i+27]-48)/((float)(ten_x(i+1)));
  *north=0;
  for(int i=0;i<5;i++)*north+=(gga[i+11]-48)*ten_x(4-i);
  for(int i=0;i<2;i++)*north+=(gga[i+17]-48)/((float)(ten_x(i+1)));
  *speed=vtg[19]-48+(vtg[21]-48)/((float)(10));
  *dire=vtg[1]-48+(vtg[3]-48)/((float)(10));
  return 0;
}

int write_sd(int fd,long time,long date,int sats,float east,float north,float speed,float dire){
  //28byte
  char buf[25];
  buf[0]=uint_2_char60(date-(date/100)*100);
  for(int i=0;i<3;i++)buf[i+1]=uint_2_char60(time/ten_x(4-i*2)-(time/ten_x(6-i*2))*100);
  buf[4]=uint_2_char60(sats);
  //east:120-150
  //north:20-50
  east-=120;
  north-=20;
  buf[5]=uint_2_char60((int)(east));
  for(int i=0;i<6;i++)buf[6+i]=east*ten_x(i+1)-((int)(east))*ten_x(i+1)+48;
  buf[12]=uint_2_char60((int)(north));
  for(int i=0;i<6;i++)buf[13+i]=north*ten_x(i+1)-((int)(north))*ten_x(i+1)+48;
  buf[19]=(int)(speed)+48;
  buf[20]=speed*10-((int)(speed))*ten_x(10)+48;
  buf[21]=(int)(speed)+48;
  buf[22]=speed*10-((int)(speed))*ten_x(10)+48;
  buf[23]='\r';
  buf[24]='\n';
  return sd_write(fd, buf, 25);
}

void show_map(float east,float north){
  for(int i=0;i<70;i++){
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
