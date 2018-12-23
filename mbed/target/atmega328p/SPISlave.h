//Author: HAVRM

#ifndef SPISLAVE_H_20150625_
#define SPISLAVE_H_20150625_

class SPISlave{
	private:
	PinName mosi;
	PinName miso;
	PinName sck;
	PinName cs;
	int bit;
	int mode;
	int hz;
	int data;
	int sate;
	public:
	SPISlave(PinName nmosi=PB_3, PinName nmiso=PB_4, PinName nsck=PB_5, PinName ncs=PB_6){
		SPISlave::mosi=nmosi;
		SPISlave::miso=nmiso;
		SPISlave::sck=nsck;
		SPISlave::cs=ncs;
		DDR(nmosi,0);
		DDR(nmiso,1);
		DDR(nsck,0);
		DDR(ncs,0);
		SPCR = (1<<SPE);
		SPISlave::format();
		SPISlave::frequency();
	}
	void format(int nbit=8, int nmode=0){
		SPISlave::bit=nbit;
		SPISlave::mode=nmode;
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
	int receive(){
		return (SPCR>>SPIF)&1;
	}
	int read(){
		while(!(SPSR & (1<<SPIF)));
		return SPDR;
	}
	int reply(int s_data){
		SPDR=s_data;
		return !((SPSR>>WCOL)&1);
	}
};



#endif
