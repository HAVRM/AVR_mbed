//  Author: hiroki.mine

#ifndef DIGITALIN_H_20150401_
#define DIGITALIN_H_20150401_

class DigitalIn{
private:
	PinName p;
public:
	DigitalIn(PinName pin){
		DigitalIn::p=pin;
		DDR(pin,0);
	}
	int read(){
		PinName pin;
		pin=DigitalIn::p;
		return PIN_READ(pin);
	}
	operator int(){
		return DigitalIn::read();
	}
};



#endif /* DIGITALIN_H_ */