// Author: hiroki.mine

#ifndef MBED_IO_H_20150404_
#define MBED_IO_H_20150404_

void DDR(PinName pin,int dir){
	if(pin!=NC){
		switch(dir){
			case 0:
			if(pin<8)DDRB=DDRB&(255-(1<<pin));
			else if(pin<16)DDRC=DDRC&(255-(1<<(pin-8)));
			else if(pin<24)DDRD=DDRD&(255-(1<<(pin-16)));
			break;
			
			case 1:
			if(pin<8)DDRB=DDRB|(1<<pin);
			else if(pin<16)DDRC=DDRC|(1<<(pin-8));
			else if(pin<24)DDRD=DDRD|(1<<(pin-16));
			break;
			
			default:
			break;
		}		
	}
}

void PORT_OUT(PinName pin,int i){
	if(pin==NC);
	else if(pin<8)PORTB=(PORTB&(255-(1<<pin)))|(i<<pin);
	else if(pin<16)PORTC=(PORTC&(255-(1<<(pin-8))))|(i<<(pin-8));
	else if(pin<24)PORTD=(PORTD&(255-(1<<(pin-16))))|(i<<(pin-16));
}

int PORT_READ(PinName pin){
	int i;
	if(pin==NC)i=0;
	else if(pin<8)i=(PORTB>>pin)&1;
	else if(pin<16)i=(PORTC>>(pin-8))&1;
	else if(pin<24)i=(PORTD>>(pin-16))&1;
	return i;
}

int PIN_READ(PinName pin){
	int i;
	if(pin==NC)i=0;
	else if(pin<8)i=(PINB>>pin)&1;
	else if(pin<16)i=(PINC>>(pin-8))&1;
	else if(pin<24)i=(PIND>>(pin-16))&1;
	return i;
}

#endif
