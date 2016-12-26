//  Author: hiroki.mine

#ifndef BUSIN_H_20150404_
#define BUSIN_H_20150404_

class BusIn{
private:
	PinName pin[16];
	int n;
public:
	BusIn (PinName p0,PinName p1=NC, PinName p2=NC, PinName p3=NC,
		   PinName p4=NC, PinName p5=NC, PinName p6=NC, PinName p7=NC,
		   PinName p8=NC, PinName p9=NC, PinName p10=NC, PinName p11=NC,
		   PinName p12=NC, PinName p13=NC, PinName p14=NC, PinName p15=NC)
	{
		BusIn::pin[0]=p0;BusIn::pin[1]=p1;BusIn::pin[2]=p2;
		BusIn::pin[3]=p3;BusIn::pin[4]=p4;BusIn::pin[5]=p5;
		BusIn::pin[6]=p6;BusIn::pin[7]=p7;BusIn::pin[8]=p8;
		BusIn::pin[9]=p9;BusIn::pin[10]=p10;BusIn::pin[11]=p11;
		BusIn::pin[12]=p12;BusIn::pin[13]=p13;BusIn::pin[14]=p14;
		BusIn::pin[15]=p15;
		PinName temp;
		int num;
		for(int i=0;i<16;i++){
			temp=BusIn::pin[i];
			if(temp!=NC)DDR(temp,0);
			else break;
			num=i+1;
		}
		BusIn::n=num;
	}
	int read(){
		int i=0;
		PinName temp;
		int num;
		num=BusIn::n;
		for(int j=0;j<num;j++){
			temp=BusIn::pin[j];
			i=(i<<1)|PIN_READ(temp);
		}
		return i;
	}
	operator int(){
		return BusIn::read();
	}		
};



#endif
