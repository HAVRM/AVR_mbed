//  Author: hiroki.mine 


#ifndef PINNAME_H_20150401_
#define PINNAME_H_20150401_

typedef enum{
	PB_0=0,
	PB_1,
	PB_2,
	PB_3,
	PB_4,
	PB_5,
	PB_6,
	PB_7,
	PC_0,
	PC_1,
	PC_2,
	PC_3,
	PC_4,
	PC_5,
	PC_6,
	PC_7,
	PD_0,
	PD_1,
	PD_2,
	PD_3,
	PD_4,
	PD_5,
	PD_6,
	PD_7,
	NC=-1
} PinName;
#define USBTX PD_1
#define USBRX PD_0

#endif