#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include <pigpio.h>

#include "signal_detector.hpp"
#include "ml808gx.hpp"

static int timeout = 0;  // 1ms
static int status = 0;
static uint32_t prevTick=0;

static void _cb(int gpio, int level, uint32_t tick, void *user) {
    ML808GX* dispenser = (ML808GX*)(user);
#if 0
    switch(level) {
        case PI_TIMEOUT:
            // STOP dispenser, if it's not stopped yet
            if(status == 1)
                //dispenser->StopDispense();
                fprintf(stderr, "PWM stopped xxxxxx!, AT: %d us, last seen in %d us before\n", tick, tick - prevTick);
            status = 0;
            break;
        case PI_HIGH:
            // Start dispenser, if it's not started yet
            if(status == 0) {
                //dispenser->StartDispense();
                // fprintf(stderr, "PWM detected HIGH !, AT: %d us, last seen in %d us before\n", tick, tick - prevTick);
    		prevTick = tick;
            }
            status = 1;
            break;
        default:
            break;
    }
    // reset GPIO
    gpioSetWatchdog(gpio, timeout);
#endif
}

int signalDetectorInitial(int pinX) {
    if(gpioInitialise()<0)
        return -1;
    gpioSetMode(pinX, PI_INPUT);
    gpioSetPullUpDown(pinX, PI_PUD_UP);
    return 0;
}

int sampleWave(int pinX, int t, void* user){
    int prev_tick;
    int pinState = 0;
    unsigned start;
    unsigned end;
    int max=0;
    unsigned timeout=150;

    while(1) {
        int pin = gpioRead(pinX);
        auto tick = gpioTick();
	
	
	//if no change wait delay and check again. reset start tick for HIGH
	if(pinState == pin) {
	    if(pin == 1)
		start = tick;
	    gpioDelay(2);
	    continue;
	}
    
	//if rising edge detected start counting
	if(pin == 1) {
	    start = tick;
	    if(pinState == 0) {
	        fprintf(stderr, "start printing\n");
	        pinState = pin;
	    }
	//if LOW then check the timeout
	}else {
	    end = tick;
	    if(timeout < end-start){
	        fprintf(stderr, "range detected : %d us\n", end-start);
	        max = end-start;
	        pinState = pin;
	    }
	}
	gpioDelay(2);
    }

    return 0;
}

void enableTrigger(int pinX, int t, void* dispenser) {
    timeout = t;
    prevTick = gpioTick();
    sampleWave(pinX, timeout, dispenser);
    //gpioSetWatchdog(pinX, timeout);
    //gpioSetAlertFuncEx(pinX, _cb, dispenser);
}

void cancelTrigger(int pinX) {
    gpioSetAlertFunc(pinX, 0);
}
