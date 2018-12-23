//  Author: HAVRM

#ifndef BUSOUT_H_20150404_
#define BUSOUT_H_20150404_

class BusOut{
private:
	PinName pin[16];
	int n;
public:
	BusOut (PinName p0,PinName p1=NC, PinName p2=NC, PinName p3=NC,
			PinName p4=NC, PinName p5=NC, PinName p6=NC, PinName p7=NC,
			PinName p8=NC, PinName p9=NC, PinName p10=NC, PinName p11=NC,
			PinName p12=NC, PinName p13=NC, PinName p14=NC, PinName p15=NC)
	{
		BusOut::pin[0]=p0;BusOut::pin[1]=p1;BusOut::pin[2]=p2;
		BusOut::pin[3]=p3;BusOut::pin[4]=p4;BusOut::pin[5]=p5;
		BusOut::pin[6]=p6;BusOut::pin[7]=p7;BusOut::pin[8]=p8;
		BusOut::pin[9]=p9;BusOut::pin[10]=p10;BusOut::pin[11]=p11;
		BusOut::pin[12]=p12;BusOut::pin[13]=p13;BusOut::pin[14]=p14;
		BusOut::pin[15]=p15;
		PinName temp;
		int num;
		for(int i=0;i<16;i++){
			temp=BusOut::pin[i];
			if(temp!=NC)DDR(temp,1);
			else break;
			num=i+1;;
		}
		BusOut::n=num;
	}
	BusOut (int i,PinName p0,PinName p1=NC, PinName p2=NC, PinName p3=NC,
	PinName p4=NC, PinName p5=NC, PinName p6=NC, PinName p7=NC,
	PinName p8=NC, PinName p9=NC, PinName p10=NC, PinName p11=NC,
	PinName p12=NC, PinName p13=NC, PinName p14=NC, PinName p15=NC)
	{
		BusOut::pin[0]=p0;BusOut::pin[1]=p1;BusOut::pin[2]=p2;
		BusOut::pin[3]=p3;BusOut::pin[4]=p4;BusOut::pin[5]=p5;
		BusOut::pin[6]=p6;BusOut::pin[7]=p7;BusOut::pin[8]=p8;
		BusOut::pin[9]=p9;BusOut::pin[10]=p10;BusOut::pin[11]=p11;
		BusOut::pin[12]=p12;BusOut::pin[13]=p13;BusOut::pin[14]=p14;
		BusOut::pin[15]=p15;
		PinName temp;
		int num;
		for(int j=0;j<16;j++){
			temp=BusOut::pin[j];
			if(temp!=NC)DDR(temp,1);
			else break;
			num=j;
		}
		BusOut::n=num;
		BusOut::write(i);
	}
	void write(int i){
		int num,temp_i;
		num=BusOut::n;
		PinName temp_p;
		for(int j=0;j<num;j++){
			temp_p=BusOut::pin[j];
			temp_i=(i>>(num-j-1))&1;
			PORT_OUT(temp_p,temp_i);
		}
	}
	int read(){
		int i=0;
		PinName temp;
		int num;
		num=BusOut::n;
		for(int j=0;j<num;j++){
			temp=BusOut::pin[j];
			i=(i<<1)|PORT_READ(temp);
		}
		return i;
	}
	BusOut& operator= (int i){
		BusOut::write(i);
		return *this;
	}
	BusOut& operator= (BusOut& i){
		BusOut::write(i.read());
		return *this;
	}
	operator int() {
		return BusOut::read();
	}
};



#endif
