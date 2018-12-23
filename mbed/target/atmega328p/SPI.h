//  Author: HAVRM

#ifndef SPI_H_20150402_
#define SPI_H_20150402_

class SPI{
private:
	PinName mosi;
	PinName miso;
	PinName sck;
	int bit;
	int mode;
	int hz;
public:
	SPI(PinName nmosi=PB_3, PinName nmiso=PB_4, PinName nsck=PB_5){
		SPI::mosi=nmosi;
		SPI::miso=nmiso;
		SPI::sck=nsck;
		DDR(nmosi,1);
		DDR(nmiso,0);
		DDR(nsck,1);
		SPCR = (1<<SPE)|(1<<MSTR);
		SPI::format();
		SPI::frequency();
	}
	void format(int nbit=8, int nmode=0){
		SPI::bit=nbit;
		SPI::mode=nmode;
		SPCR=(SPCR&243)|(nmode<<CPHA);
	}
	int frequency(long hz = 1000000){
		int rate,spr,x2;
		rate=F_CPU/hz;
		if(rate<=3){rate=2;spr=0;x2=0;}
		else if(rate<=6){rate=4;spr=0;x2=1;}
		else if(rate<=12){rate=8;spr=1;x2=0;}
		else if(rate<=24){rate=16;spr=1;x2=1;}
		else if(rate<=48){rate=32;spr=2;x2=0;}
		else if(rate<=96){rate=64;spr=3;x2=0;}
		else{rate=128;spr=3;x2=1;}
		SPCR = (SPCR&252)|spr;
		SPSR=(SPSR&254)|x2;
		return F_CPU/rate;
	}
	uint8_t write(uint8_t data){
		SPDR=data;
		while(!(SPSR & (1<<SPIF)));
		return SPDR;
	}
	
};



#endif
