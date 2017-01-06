// Author: hiroki.mine
// This is not for Atmega88



#ifndef SERIAL_H_20150516_2043_
#define SERIAL_H_20150516_2043_

class Serial{
private:
	
	PinName tx;
	PinName rx;
	int baudrate,rate,bits,stop_bits;
	int keta[5];
	
	void reset_keta(){
		keta[0]=0;
		keta[1]=0;
		keta[2]=0;
		keta[3]=-1;
		keta[4]=-1;
	}

public:
	
	void print_keta(){
		Serial::printf("%d,%d,%d,%d,%d\n\r",Serial::keta[0],Serial::keta[1],Serial::keta[2],Serial::keta[3],Serial::keta[4]);
	}
	
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
		Serial::reset_keta();
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
		UCSR0A=UCSR0A|(1<<1);
		UCSR0B=UCSR0B|(1<<RXEN0)|(1<<TXEN0);
		UCSR0C=UCSR0C|(npr<<UPM00)|(nst<<USBS0)|((8-5)<<UCSZ00);		
	}
	
	char getc(){
		while(!UCSR0A&(1<<RXC0));
		return UDR0;
	}
	
	void scanf(const char *fmt, ...){
		int *seriali;
		unsigned int *serialui;
		long *seriall;
		unsigned long *serialul;
		float *serialf;
		double *seriald;
		va_list serialarg;
		va_start(serialarg,fmt);
		char serialc=*fmt;
		while(serialc!='\0'){
			if(serialc!='%')while(Serial::getc()!=serialc);
			else{
				fmt++;
				serialc=*fmt;
				switch(serialc){
					case '%':
						while(Serial::getc()!='%');
						break;
					
					case 'd':
						seriali=va_arg(serialarg,int*);
						*seriali=0;
						serialc=Serial::getc();
						while((int)serialc>=48 && (int)serialc<=57)*seriali=*seriali*10+(int)serialc-48;
						break;
					
					case 'f':
						int state=0;
						serialf=va_arg(serialarg,float*);
						*serialf=0;
						serialc=Serial::getc();
						while(1){
							if((int)serialc>=48 && (int)serialc<=57){
								if(state==0)*serialf=*serialf*10+(int)serialc-48;
								else{
									*serialf=*serialf+((int)serialc-48)/((float)(10^state));
									state++;
								}								
							}else if(state==0 && serialc=='.'){
								state=1;
							}else break;
						}
						
				}
			}
			fmt++;
			serialc=*fmt;
		}
	}
	
	int readable(){
		return UCSR0A&(1<<RXC0);
	}
	
	void putc(unsigned char data){
		while(!UCSR0A&(1<<UDRE0));
		UDR0=data;
		_delay_ms(1.5);
	}
	
	void puts(char *str){
		char *t_str=str;
		if(keta[3]!=-1){
			keta[4]=0;
			while(*t_str!='\0'){
				keta[4]++;
				t_str++;
			}		
			if(keta[1]==0){
				while(keta[3]>keta[4]){
					Serial::putc(' ');
					keta[3]--;
				}				
			}
		}
		while(*str!='\0'){
			Serial::putc(*str);
			str++;
		}
		if(keta[1]==1){
			while(keta[3]>keta[4]){
				Serial::putc(' ');
				keta[3]--;
			}
		}		
	}
	
	void putn(long num,int n=10,int miner=0){
		char l[100];
		if(num<0){
			num*=-1;
			miner=1;
		}		
		keta[4]=log10(num)/log10(n);
		keta[4]++;
		int i=0;
		if(keta[0]==1 || miner==1)keta[4]++;
		if(keta[1]==0 && keta[3]!=-1){
			while(keta[4]<keta[3]){
				if(keta[2]==0)Serial::putc(' ');
				else Serial::putc('0');
				keta[3]--;
			}
		}
		if(miner==1)Serial::putc('-');
		else if(keta[0]==1)Serial::putc('+');
		if(num==0)Serial::putc('0');
		else{
			while(num>0){
				l[i]=num%n+48;
				num/=n;
				if(l[i]>57){
					switch(l[i]-48){
						case 10:
							l[i]='a';
							break;
						
						case 11:
							l[i]='b';
							break;
						
						case 12:
							l[i]='c';
							break;
						
						case 13:
							l[i]='d';
							break;
						
						case 14:
							l[i]='e';
							break;
						
						case 15:
							l[i]='f';
							break;
						
						default:
							break;
					}
				}
				i++;
			}
			for(int j=i-1;j>=0;j--)Serial::putc(l[j]);
		}
		if(keta[1]==1 && keta[3]!=-1){
			while(keta[4]<keta[3]){
				Serial::putc(' ');
				keta[3]--;
			}
		}			
	}
	
	void putlf(double num){
		int t_k[5],miner=0;
		t_k[0]=keta[0];
		t_k[1]=keta[1];
		t_k[2]=keta[2];
		t_k[3]=keta[3];
		t_k[4]=keta[4];
		if(num<0){
			miner=1;
			num*=-1;
		}
		long h=(long)num;
		long l;
		num-=h;
		if(t_k[3]!=-1 && t_k[4]==-1){
			t_k[4]=t_k[3]-(int)log10(h)-1-(miner | t_k[0]);
			keta[3]-=t_k[4];
		}
		if(t_k[4]!=-1){
			for(int i=0;i<t_k[4];i++)num*=10;
			l=num;
		}else{
			t_k[4]=0;
			while((double)l!=num){
				num*=10;
				l=(long)num;
				t_k[4]++;
			}
		}
		if(keta[1]==1)keta[3]=-1;
		else if(keta[3]!=-1 && keta[4]!=-1)keta[3]-=t_k[4];
		//else if(keta[3]!=-1)
		Serial::putn(h,10,miner);
		Serial::putc('.');
		//Serial::print_keta();
		t_k[0]=0;
		if(t_k[1]==0)t_k[3]=t_k[4];
		else if(t_k[3]!=-1)t_k[3]-=keta[4];
		keta[0]=t_k[0];
		keta[1]=t_k[1];
		keta[2]=t_k[2];
		keta[3]=t_k[3];
		keta[4]=t_k[4];
		Serial::putn(l);
	}
	
	void printf(const char *fmt, ...){
		int k_n=0;
		int seriali;
		unsigned int serialui;
		long seriall;
		unsigned long serialul;
		float serialf;
		double seriald;
		char *serials;
		va_list serialarg;
		va_start(serialarg,fmt);
		char serialc=*fmt;
		while((serialc=*fmt)!=0){
			if(serialc!='%'){
				Serial::putc(serialc);
				fmt++;
			}else{
				Serial::reset_keta();
				k_n=0;
rereade:
				fmt++;
				serialc=*fmt;
				switch(serialc){
					
					case '%':
						Serial::putc('%');
						break;
					
					case 'b':
						seriali=va_arg(serialarg,int);
						Serial::putn((long)seriali,2);
						break;
					
					case 'd':
						seriali=va_arg(serialarg,int);
						Serial::putn((long)seriali);
						break;
				
					case 'f':
						serialf=va_arg(serialarg,double);
						Serial::putlf(serialf);
						break;
					
					case 'l':
						fmt++;
						serialc=*fmt;
						switch(serialc){
							
							case 'b':
								seriali=va_arg(serialarg,long);
								Serial::putn(seriali,2);
								break;
							
							case 'd':
								seriall=va_arg(serialarg,long);
								Serial::putn(seriall);
								break;
						
							case 'f':
								seriald=va_arg(serialarg,double);
								Serial::putlf(seriald);
								break;
						
							case 'o':
								seriall=va_arg(serialarg,long);
								Serial::putn(seriall,8);
								break;
						
							case 'u':
								serialul=va_arg(serialarg,unsigned long);
								Serial::putn((long)serialul);
								break;
						
							case 'x':
								seriall=va_arg(serialarg,long);
								Serial::putn(seriall,16);
								break;
							
							default:
								break;
						}
						break;
					
					case 'o':
						seriali=va_arg(serialarg,int);
						Serial::putn((long)seriali,8);
						break;
					
					case 's':
						serials=va_arg(serialarg,char*);
						Serial::puts(serials);
						break;
					
					case 'u':
						serialui=va_arg(serialarg,unsigned int);
						Serial::putn((long)serialui);
						break;
					
					case 'x':
						seriali=va_arg(serialarg,int);
						Serial::putn((long)seriali,16);
						break;
					
					case '0':
						if(k_n<=2){
							k_n=3;
							keta[2]=1;
						}else{
							k_n=4;
							keta[4]=0;
						}
						goto rereade;
					
					case '-':
						if(k_n<=1){
							k_n=2;
							keta[1]=1;
						}
						goto rereade;
					case '+':
						if(k_n==0){
							k_n=1;
							keta[0]=1;
						}
						goto rereade;
					
					case '.':
						k_n=4;
						goto rereade;
					
					default:
						if(48<serialc && serialc<=57){
							if(k_n<=3){
								k_n=3;
								if(keta[3]==-1)keta[3]=serialc-48;
								else keta[3]=keta[3]*10+serialc-48;
							}else if(k_n==4){
								if(keta[4]==-1)keta[4]=serialc-48;
								else keta[4]=keta[4]*10+serialc-48;
							}
						}
						goto rereade;											
				}
				fmt++;				
			}
		}
	}
};

#endif