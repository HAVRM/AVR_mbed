/*
 * DigitalInOut.h
 *
 * Created: 2015/04/01 17:01:33
 *  Author: HAVRM
 */ 


#ifndef DIGITALINOUT_H_20150401_
#define DIGITALINOUT_H_20150401_

class DigitalInOut{
private:
	PinName p;
	int dir;
public:
	DigitalInOut(PinName pin){
		DigitalInOut::p=pin;
		DigitalInOut::output();
	}
	DigitalInOut(PinName pin,int d){
		DigitalInOut::p=pin;
		DigitalInOut::dir=d;
		DDR(pin,d);
	}
	void input(){
		DigitalInOut::dir=0;
		PinName pin;
		pin=DigitalInOut::p;
		DDR(pin,0);
	}
	void output(){
		DigitalInOut::dir=1;
		PinName pin;
		pin=DigitalInOut::p;
		DDR(pin,1);
	}
	int read(){
		int d;
		d=DigitalInOut::dir;
		int i=0;
		PinName pin;
		pin=DigitalInOut::p;
		if(d==0)i=PIN_READ(pin);
		else if(d==1)i=PORT_READ(pin);
		return i;
	}
	void write(int i){
		int d;
		d=DigitalInOut::dir;
		PinName pin;
		pin=DigitalInOut::p;
		if(d==1)PORT_OUT(pin,i);		
	}
	DigitalInOut& operator= (int i){
		DigitalInOut::write(i);
		return *this;
	}
	DigitalInOut& operator= (DigitalInOut& i){
		DigitalInOut::write(i.read());
		return *this;
	}
	operator int() {
		return DigitalInOut::read();
	}
};




#endif /* DIGITALINOUT_H_ */
