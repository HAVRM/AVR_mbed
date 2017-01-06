//#define USE_SERIAL_PRINTF_FULL //use full printf (cannot use @ATmega88)

#include "mbed.h"

DigitalOut led(PB_0);

int main(void){
  while(1){
    led=!led;
    wait_ms(500);
  }
}
