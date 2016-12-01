// Author: hiroki.mine

#ifndef MBED_WAIT_H_20161130_1831_
#define MBED_WAIT_H_20161130_1831_

void wait_ms(int ms){
  for(int i=0;i<ms;i++)_delay_ms(1);
}

void wait(double ms){
  wait_ms(ms*1000);
}

void wait_us(int us){
  for(int i=0;i<us;i++)_delay_us(1);
}

#endif
