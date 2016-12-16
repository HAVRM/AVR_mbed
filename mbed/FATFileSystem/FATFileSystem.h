//  Author: hiroki.mine

#ifndef FATFILESYSTEM_H_20161202_
#define FATFILESYSTEM_H_20161202_

#include "diskio.h"
#include "ff.h"
#include "ffconf.h"
#include "integer.h"
#include <stddef.h>

/**
 * FATFileSystem based on ChaN's Fat Filesystem library v0.8 
 */

int _USED_FATFS[9]={0};
char _FATFS_NAME[9][8];

class FATFileSystem{
public:
    FATFS _fs;
    FIL _fil;
    DIR _dir;
    BYTE _work[_MAX_SS];
    int _used_fatfs_num;
    char _fsid[2];

    FATFileSystem(){;}

    FATFileSystem(const char* n);

    ~FATFileSystem(){
      _USED_FATFS[this->_used_fatfs_num]=0;
      f_mount(NULL,this->_fsid,0);
    }

    FRESULT open(const char* name, const char* flags){
      BYTE openmode;
      char* n;
      int monum=0,j=0;
      for(j=0;name[j]!='\0';j++);
      n=(char *)malloc(sizeof(char)*(j+5));
      for(int i=0;i<2;i++)n[i]=this->_fsid[i];
      n[2]=':';
      n[3]='/';
      for(int i=0;i<=j;i++)n[i+4]=name[i];
      for(int i=0;flags[i]!='\0';i++){
        if(flags[i]=='w')monum|=0x01;
        if(flags[i]=='r')monum|=0x02;
        if(flags[i]=='a')monum|=0x04;
      }
      if((monum>>2)==1)openmode=FA_OPEN_APPEND;
      else if(monum==3)openmode=FA_READ|FA_WRITE|FA_CREATE_ALWAYS;
      else if(monum==2)openmode=FA_READ|FA_OPEN_EXISTING;
      else if(monum==1)openmode=FA_WRITE|FA_CREATE_ALWAYS;
      return f_open(&this->_fil,n,openmode);
    }

    int remove(const char *filename){return f_unlink(filename);}
    
    int rename(const char *oldname, const char *newname){return f_rename(oldname,newname);}
    
    int format(){return f_mkfs(this->_fsid,FM_ANY,0,this->_work,512);}
    
    int opendir(const char *name){return f_opendir(&this->_dir,name);}
    
    int mkdir(const char *name){return f_mkdir(name);}

    int mount(){return f_mount(&this->_fs,this->_fsid,1);}

    int unmount(){
      return f_mount(NULL,this->_fsid,0);
    }
};

FATFileSystem _fattemp[9];

FATFileSystem::FATFileSystem(const char* n){
  for(int i=0;i<9;i++){
    if(_USED_FATFS[i]==0){
      _USED_FATFS[i]=1;
      this->_used_fatfs_num=i;
      this->_fsid[0]='0'+i;
      this->_fsid[1]='\0';
      f_mount(&this->_fs,this->_fsid,0);
      for(int j=0;j<8;j++)_FATFS_NAME[i][j]='\0';
      for(int j=0;j<8;j++)_FATFS_NAME[i][j]=n[j];
      _fattemp[i]=*this;
      return;
    }
  }
}

#include "fatio.h"

#endif
