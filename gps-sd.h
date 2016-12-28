//  Author: hiroki.mine

#ifndef GPS_SD_H_20161224_
#define GPS_SD_H_20161224_
#include "jmap.h"
#include "sd.h"
#include "serial.h"
#include "mbed.h"

union int_and_float{
  int ival;
  float fval;
};

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

long ten_x(int a){ //10^a
  int ans=1;
  for(int i=0;i<a;i++)ans*=10;
  return ans;
}

char uint_2_char60(int data){
  data%=60;
  if(data<0)return '@';
  if(data<10)return '0'+data;
  else if(data<36)return 'a'+data-10;
  else return 'A'+data-36;
}

char uint_2_char60_2(int data){
  data%=36;
  if(data<0)return '@';
  if(data<10)return '0'+data;
  else if(data<36)return 'a'+data-10;
  else return '_';
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

int open_sd(int date[3],int time[3],int j){ //date:yy,mm,dd time:hh,mm,ss number
  int fd;
  char fname[]="dhmm0.txt";
  char buf[13];
  buf[12]='\n';
  fname[0]=uint_2_char60_2(date[2]-1);
  fname[1]=uint_2_char60_2(time[0]);
  fname[2]=uint_2_char60_2(time[1]/30);
  fname[3]=uint_2_char60_2(time[1]%30);
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

int get_gps(int time[3],int date[3],int *sats,float *east,float *north,float *speed,float *dire,int fd){ //get->return 0
  int i,temp;
  char c;
  char gga[100], vtg[100], zda[100], gll;
  int n=0,k=0; //n is numbers of message, k is numbers in messege
  int get_id=0;
  int ch=0;
  while(ch!=0b1111){
    c=USART_getc();
    row=~64;
    if(sw==0b10)col=4;
    else col=0;
    while(c!='$')c=USART_getc();
    if((c=USART_getc())=='G')if((c=USART_getc())=='P'){
      if((c=USART_getc())=='G'){
        if((c=USART_getc())=='G'){
          if((c=USART_getc())=='A')if((c=USART_getc())==','){
            c=USART_getc();
            row=~2;
            for(i=0; c!='*' && c!='\n' && i<100;i++){
              gga[i]=c;
              c=USART_getc();
            }
            ch|=1;
          }
        }
        else if(c=='L')if((c=USART_getc())=='L')if((c=USART_getc())==','){
          c=USART_getc();
          row=~4;
          for(i=0; c!='*' && c!='\n' && i<100;i++){
            gll=c;
            c=USART_getc();
          }
          ch|=2;
        }
      }
      else if(c=='V'){
        if((c=USART_getc())=='T')if((c=USART_getc())=='G')if((c=USART_getc())==','){
         c=USART_getc();
         row=~8;
         for(i=0; c!='*' && c!='\n' && i<100;i++){
            vtg[i]=c;
            c=USART_getc();
          }
          ch|=4;
        }
      }
      else if(c=='Z')if((c=USART_getc())=='D')if((c=USART_getc())=='A')if((c=USART_getc())==','){
        c=USART_getc();
        row=~16;
        for(i=0; c!='*' && c!='\n' && i<100;i++){
          zda[i]=c;
          c=USART_getc();
        }
        ch|=8;
      }
    }
  }
  row=~1;
  if(gll!='A')return -1;
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
  *speed=get_float_from_nmea(vtg,7);
  *dire=get_float_from_nmea(vtg,0);
  /*for(i=0;i<3;i++){
    zda[i*2]=time[i]/10+48;
    zda[i*2+1]=time[i]%10+48;
  }
  sd_write(fd, zda, 6);
  for(i=0;i<3;i++){
    zda[i*2]=date[i]/10+48;
    zda[i*2+1]=date[i]%10+48;
  }
  sd_write(fd, zda, 6);
  zda[0]=*sats/10+48;
  zda[1]=*sats%10+48;
  sd_write(fd, zda, 2);
  zda[0]=(*east-130)/10+48;
  zda[1]=((int)(*east-130))%10+48;
  sd_write(fd, zda, 2);
  zda[0]=(*north-20)/10+48;
  zda[1]=((int)(*north-20))%10+48;
  sd_write(fd, zda, 2);
  zda[0]=((int)(*speed))+48;
  zda[1]=((int)(*speed*10))%10+48;
  sd_write(fd, zda, 2);
  zda[0]=((int)(*dire))+48;
  zda[1]=((int)(*dire*10))%10+48;
  sd_write(fd, zda, 2);
  zda[0]='\r';
  zda[1]='\n';
  sd_write(fd, zda, 2);*/
  row=~1;
  return 0;
}

int write_sd(int fd,int time[3],int date[3],int sats,float east,float north,float speed,float dire){
  //23byte
  char buf[25];
  int temp=0;
  buf[0]=uint_2_char60(date[2]);
  for(int i=0;i<3;i++)buf[i+1]=uint_2_char60(time[i]);
  buf[4]=uint_2_char60(sats);
  long tle8=(long)((east-120)*1000000);//:0-30000000
  long tln8=(long)((north-20)*1000000);//:0-30000000
  long tls6=(long)(speed*100);//:0-100000
  long tld5=(long)(dire*100);//:0-36000
  for(int i=0;i<5;i++){
    buf[i+5]=uint_2_char60(tle8%60);
    tle8/=60;
  }
  for(int i=0;i<5;i++){
    buf[i+10]=uint_2_char60(tln8%60);
    tln8/=60;
  }
  for(int i=0;i<3;i++){
    buf[i+15]=uint_2_char60(tls6%60);
    tls6/=60;
  }
  for(int i=0;i<3;i++){
    buf[i+18]=uint_2_char60(tld5%60);
    tld5/=60;
  }
  buf[21]='\r';
  buf[22]='\n';
  return sd_write(fd, buf, 23);
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
