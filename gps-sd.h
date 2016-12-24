//  Author: hiroki.mine

#ifndef GPS_SD_H_20161224_
#define GPS_SD_H_20161224_

#include "jmap.h"

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

#endif
