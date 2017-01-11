#include "stdio.h"

int main(int arg,char *argv[]){
  if(arg!=2)return 0;
  FILE *fp;
  if((fp=fopen(argv[1],"r"))==NULL){
    pritnf("can't open file\n\r");
    return 0;
  }
      
