PROG = Hello_World
MMCU = atmega328p
CPUFREQ = 1000000

#change when use AVRISP mkII
PORT = usb
TARGET = m328p
MEMORY = flash
TYPE = w

CC = avr-gcc
CXX = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

all : clean $(PROG).ihex

$(PROG).ihex : $(PROG)
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

$(PROG) : $(PROG).o
	$(CC) -g -O2 -mmcu=$(MMCU) $< -o $@

.c.o :
	$(CC) -I./mbed -I./mbed/target/$(MMCU) -g -O2 -mmcu=$(MMCU) -DF_CPU=$(CPUFREQ)UL -c -o $@ $<
.cpp.o :
	$(CXX) -I./mbed -I./mbed/target/$(MMCU) -g -O2 -mmcu=$(MMCU) -DF_CPU=$(CPUFREQ)UL -c -o $@ $<

.PHONY: wmk
wmk :
	sudo $(AVRDUDE) -c avrispmkII -P $(PORT) -p $(TARGET) -U $(MEMORY):$(TYPE):$(PROG).ihex

.PHONY: write
write : all w

.PHONY: w
w :
	hidspx $(PROG).ihex


.PHONY: r
r :
	hidspx -r

.PHONY: clean
clean :
	$(RM) *.o *.ihex $(PROG) *~
