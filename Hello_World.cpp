#include "mbed/mbed.h"

DigitalOut LED(PB_0);
int main(void){
  LED=0;
  _delay_ms(500);
  LED=1;
}
