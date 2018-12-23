//  Author: HAVRM


#ifndef DIGITALOUT_H_20150401_
#define DIGITALOUT_H_20150401_


class DigitalOut{
private:
	PinName p;
public:
	DigitalOut(PinName pin=NC){
		DigitalOut::p=pin;
		DDR(pin,1);
	}
	DigitalOut(PinName pin,int i){
		DigitalOut::p=pin;
		DDR(pin,1);
		PORT_OUT(pin,i);		
	}
	void write(int i){
		PinName pin;
		pin=DigitalOut::p;
		PORT_OUT(pin,i);		
	}
	int read(){
		PinName pin;
		pin=DigitalOut::p;
		return PORT_READ(pin);
	}
	DigitalOut& operator= (int i){
		DigitalOut::write(i);
		return *this;
	}
	DigitalOut& operator= (DigitalOut& i){
		DigitalOut::write(i.read());
		return *this;
	}
	operator int() {
		return DigitalOut::read();
	}
};



#endif
