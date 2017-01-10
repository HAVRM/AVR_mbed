//  Author: hiroki.mine

#ifndef GPS_H_20161224_
#define GPS_H_20161224_
#include "serial.h"
#include "mbed.h"

long ten_x(int a){ //10^a
  int ans=1;
  for(int i=0;i<a;i++)ans*=10;
  return ans;
}

char get_char_from_nmea(char *data,int num){
  int read=-1;
  for(int i=0;i<num;i++){
    read++;
    for(int j=0;data[read]!=',';j++){
      if(data[j]=='\0')return 0;
      read++;
    }
  }
  return data[read+1];
}

int get_int_from_nmea(char *data,int num){
  int ans=0,read=-1;
  for(int i=0;i<num;i++){
    read++;
    for(int j=0;data[read]!=',';j++){
      if(data[read]=='\0')return 0;
      read++;
    }
  }
  read++;
  for(int i=0;data[read+i]!=',';i++){
    if(data[i+read]=='\0')return ans;
    ans=ans*10+(data[i+read]-48);
  }
  return ans;
}

long get_long_from_nmea(char *data,int num){
  long ans=0;
  int read=-1;
  for(int i=0;i<num;i++){
    read++;
    for(int j=0;data[read]!=',';j++){
      if(data[read]=='\0')return 0;
      read++;
    }
  }
  read++;
  for(int i=0;data[read+i]!=',';i++){
    if(data[i+read]=='\0')return ans;
    ans=ans*10+(data[i+read]-48);
  }
  return ans;
}

float get_float_from_nmea(char *data,int num){
  float ans=0;
  int read=-1;
  for(int i=0;i<num;i++){
    read++;
    for(int j=0;data[read]!=',';j++){
      if(data[read]=='\0')return 0;
      read++;
    }
  }
  read++;
  for(int i=0;data[read]!='.';i++){
    if(data[read]=='\0')return ans;
    ans=ans*10+(data[read]-48);
    read++;
  }
  read++;
  for(int i=0;data[read+i]!=',';i++){
    if(data[i+read]=='\0')return ans;
    ans=ans+(data[i+read]-48)/((float)(ten_x(i+1)));
  }
  return ans;
}

void get_now(int date[3],int time[3]){
  char c;
  int ch=0;
  char temp[50];
  while(1){
    c=USART_getc();
    while(c!='$')c=USART_getc();
    if(USART_getc()=='G')if(USART_getc()=='P')if(USART_getc()=='Z')if(USART_getc()=='D')if(USART_getc()=='A')if(USART_getc()==','){
      for(int i=0;i<50;i++)temp[i]=USART_getc();
      ch=1;
    }
    if(ch!=0)break;
  }
  time[0]=((long)(get_float_from_nmea(temp,0)))/10000;
  time[1]=((long)(get_float_from_nmea(temp,0)))/100-time[0]*100;
  time[2]=((long)(get_float_from_nmea(temp,0)))-time[0]*10000-time[1]*100;
  date[2]=get_int_from_nmea(temp,1);
  date[1]=get_int_from_nmea(temp,2);
  date[0]=get_int_from_nmea(temp,3);
  date[0]=date[0]%100;
} 

int get_gps(int time[3],int date[3],int *sats,float *east,float *north,float *high,float *speed,float *dire){ //get->return 0
  int i,temp;
  char c;
  char gga[100], vtg[100], zda[100], gll;
  int n=0,k=0; //n is numbers of message, k is numbers in messege
  int get_id=0;
  while(1){
    c=USART_getc();
    while(c!='$')c=USART_getc();
    if((c=USART_getc())=='G')if((c=USART_getc())=='P'){
      if((c=USART_getc())=='G'){
        if((c=USART_getc())=='G'){
          if((c=USART_getc())=='A')if((c=USART_getc())==','){
            c=USART_getc();
            for(i=0; c!='*' && c!='\n' && i<100;i++){
              gga[i]=c;
              c=USART_getc();
            }
          }
        }
        else if(c=='L')if((c=USART_getc())=='L')if((c=USART_getc())==','){
          c=USART_getc();
          for(i=0; c!='*' && c!='\n' && i<100;i++){
            gll=c;
            c=USART_getc();
          }
        }
      }
      else if(c=='V'){
        if((c=USART_getc())=='T')if((c=USART_getc())=='G')if((c=USART_getc())==','){
         c=USART_getc();
         for(i=0; c!='*' && c!='\n' && i<100;i++){
            vtg[i]=c;
            c=USART_getc();
          }
        }
      }
      else if(c=='Z')if((c=USART_getc())=='D')if((c=USART_getc())=='A')if((c=USART_getc())==','){
        c=USART_getc();
        for(i=0; c!='*' && c!='\n' && i<100;i++){
          zda[i]=c;
          c=USART_getc();
        }
        break;
      }
    }
  }
  time[0]=((long)(get_float_from_nmea(zda,0)))/10000;
  time[1]=((long)(get_float_from_nmea(zda,0)))/100-time[0]*100;
  time[2]=((long)(get_float_from_nmea(zda,0)))-time[0]*10000-time[1]*100;
  date[2]=get_int_from_nmea(zda,1);
  date[1]=get_int_from_nmea(zda,2);
  date[0]=get_int_from_nmea(zda,3);
  date[0]=date[0]%100;
  *sats=get_int_from_nmea(gga,6);
  *east=get_float_from_nmea(gga,3)/((float)(100));
  *north=get_float_from_nmea(gga,1)/((float)(100));
  *high=get_float_from_nmea(gga,8);
  *speed=get_float_from_nmea(vtg,7);
  *dire=get_float_from_nmea(vtg,0);
  if(gll!='A')return -1;
  return 0;
}

#endif
