// Author: hiroki.mine



#ifndef SERIAL_H_20150516_2043_
#define SERIAL_H_20150516_2043_

class Serial{
	private:
	
	PinName tx;
	PinName rx;
	int baudrate,rate,bits,stop_bits;
	
	public:
	
	enum Parity {
		None = 0,
		Odd,
		Even,
		Forced1,
		Forced0
	};
	
	enum IrqType {
		RxIrq = 0,
		TxIrq
	};
	
	Parity parity;
	
	Serial(PinName ntx=USBTX,PinName nrx=USBRX){
		DDR(ntx,0);
		DDR(nrx,0);
		Serial::baud();
		Serial::format();
	}
	
	void baud(unsigned int nbr=9600,Parity parity=Serial::None,int nst=1){
		Serial::baudrate=1000000/8/nbr-1;
		Serial::rate=nbr;
		UBRR0H=(unsigned char)(Serial::baudrate>>8);
		UBRR0L=(unsigned char)(Serial::baudrate);
	}
	
	void format(int nbit=8,Parity npr=Serial::None,int nst=1){
		if(nbit>8)nbit=8;
		if(nst>2)nst=2;
		else if(nst<1)nst=1;
		Serial::bits=nbit;
		Serial::parity=npr;
		Serial::stop_bits=nst;
		UCSR0A=1<<U2X0;
		UCSR0B=(1<<RXEN0)|(1<<TXEN0);
		UCSR0C=(npr<<UPM00)|((nst-1)<<USBS0)|(3<<UCSZ00);
	}
	
	unsigned char getc(){
		while(!UCSR0A&(1<<RXC0));
		return UDR0;
	}
	
	int readable(){
		return UCSR0A&(1<<RXC0);
	}
	
	void putc(unsigned char data){
		while(!UCSR0A&(1<<UDRE0));
		UDR0=data;
		_delay_ms(1.5);
	}
	
	void putn(long num){
		char l[100];
		int i=0;
		if(num<0){
			num*=-1;
			Serial::putc('-');
		}
		while(num>0){
			l[i]=num%10+48;
			num/=10;
			i++;
		}
		for(int j=i-1;j>=0;j--)Serial::putc(l[j]);
	}
	
	void putlf(double num){
		long h=(long)num;
		long l;
		num-=h;
		while((double)l!=num){
			num*=10;
			l=(long)num;
		}
		Serial::putn(h);
		Serial::putc('.');
		Serial::putn(l);
	}
	
	void printf(const char *fmt, ...){
		int seriali;
		float serialf;
		va_list serialarg;
		va_start(serialarg,fmt);
		char serialc=*fmt;
		while((serialc=*fmt)!=0){
			if(serialc!='%'){
				Serial::putc(serialc);
				fmt++;
			}else{
				fmt++;
				serialc=*fmt;
				switch(serialc){
					
					case '%':
					Serial::putc('%');
					break;
					
					case 'd':
					seriali=va_arg(serialarg,int);
					Serial::putn((long)seriali);
					break;
					
					case 'f':
					serialf=va_arg(serialarg,double);
					Serial::putlf(serialf);
					break;
					
				}
				fmt++;
			}
		}
	}
};

#endif
