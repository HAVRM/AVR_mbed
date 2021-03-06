//  Author: HAVRM
 

#ifndef MBED_H_20150401_
#define MBED_H_20150401_
#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdarg.h>
#include <math.h>

#include "PinName.h"

#include "mbed_IO.h"

#include "mbed_wait.h"

#include "DigitalOut.h"
#include "DigitalIn.h"
#include "DigitalInOut.h"
#include "BusIn.h"
#include "BusOut.h"

#ifdef USE_SERIAL_PRINTF_FULL
#include "Serial.h"
#else
#include "Serial_lite.h"
#endif

#include "SPI.h"
#include "SPISlave.h"




#endif
