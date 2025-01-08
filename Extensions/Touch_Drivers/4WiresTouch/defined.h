#if defined(__SAMD21G18A__)
//
#else
#define USE_FAST_PINIO
#endif

#include "User_Setup.h"

#if defined(ARDUINO_ARCH_AVR) || defined(ESP32) || defined(CONFIG_IDF_TARGET_ESP32C6)
    #include <pgmspace.h>
#else
    #include <avr/pgmspace.h>
#endif


#define NUMSAMPLES 2		// sample number
#define COMP       5
#define AVERAGE    1
#define RXPLATE    300

unsigned char _yp = YP;
unsigned char _xm = XM;
unsigned char _ym = YM;
unsigned char _xp = XP;

#ifdef AVERAGE
#define AVERAGETIME 3
static int avr_analog(int adpin) {
    int sum = 0;
    int max = 0;
    int min = 1024;
    for (int i = 0; i < AVERAGETIME; i++) {
        int tmp = analogRead(adpin);
        if (tmp > max) {
            max = tmp;
        }
        if (tmp < min) {
            min = tmp;
        }
        sum += tmp;
        //   sum+=analogRead(adpin);
    }
    return (sum - min - max) / (AVERAGETIME - 2);

}

#endif