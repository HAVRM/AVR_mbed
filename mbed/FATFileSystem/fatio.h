#ifndef FATIO_H_20161217_
#define FATIO_H_20161217_

#include "FATFileSystem.h"
#include <stdlib.h>

typedef FATFileSystem FILE;

int _find_fatfs_id(const char* name, int num=0){
  char *n;
  int j;
  for(j=1;name[j]!='/' && name[j]!='\0';j++);
  n=(char *)malloc(sizeof(char)*(j-1));
  for(int i=0;i<j-1;i++)n[i]=name[i+1];
  for(int i=0;i<9;i++){
    for(int k=0;k<8;k++){
      if(k>0 && _FATFS_NAME[i][k]=='\0')break;
      if(name[k]!=_FATFS_NAME[i][k])goto nextid;
    }
    return i;
nextid:
    ;
  }
  if(num>3)return -1;
  FATFileSystem _ffi_temp(name);
  num++;
  return _find_fatfs_id(name,num);
}

FILE* fopen(const char* name, const char* flags){
  int num=_find_fatfs_id(name);
  char *n;
  int j,k;
  for(j=1;name[j]!='/';j++);
  for(k=j;name[k]!='\0';k++);
  n=(char *)malloc(sizeof(char)*(k-j));
  for(int i=0;i<k-j;i++)n[i]=name[j+i];
  _fattemp[num].open(name,flags);
  return &_fattemp[num];
}

int fclose(FILE *fp){return f_close(&fp->_fil);}

int fputc(char data,FILE *fp){return f_putc(data,&fp->_fil);}
int fputs(const char* data,FILE *fp){return f_puts(data,&fp->_fil);}

char* fgets(char *s,int n,FILE *fp){return f_gets(s,n,&fp->_fil);}

/*int fprintf(FILE *fp,const char* fmt,...){
  char tmp[512];
  int i;
  va_list arg;
  va_start(arg,fmt);
  //sprintf(tmp,fmt,arg);
  va_end(arg);
  for(i=0;tmp[i]!='\0';i++);
  f_printf(&fp->_fil,tmp);
  return i;
}*/


#endif
