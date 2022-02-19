#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include <pigpio.h>

#include "signal_dector.hpp"
#include "ml808gx.hpp"

static int timeout = 1;  // 1ms
static int status = 0;
static uint32_t prevTick=0;

static void _cb(int gpio, int level, uint32_t tick, void *user) {
    ML808GX* dispenser = (ML808GX*)(user);
    switch(level) {
        case PI_TIMEOUT:
            // STOP dispenser, if it's not stopped yet
            if(status == 1)
                // dispenser->StopDispense();
                fprintf(stderr, "PWM stopped!, AT: %d us, last seen in %d us before", tick, tick - prevTick);
            status = 0;
            break;
        case PI_HIGH:
            // Start dispenser, if it's not started yet
            if(status == 0) {
                // dispenser->StartDispense();
                fprintf(stderr, "PWM detected!, AT: %d us, last seen in %d us before", tick, tick - prevTick);
            }
            status = 1;
            break;
        default:
            break;
    }
    prevTick = tick;
    // reset GPIO
    gpioSetWatchdog(gpio, timeout);
}

int signalDectorInitial(int pinX) {
    if(gpioInitialise()<0)
        return -1;
    gpioSetMode(pinX, PI_INPUT);
    gpioSetPullUpDown(pinX, PI_PUD_UP);

    return 0;
}

void enableTrigger(int pinX, int t, void* dispenser) {
    timeout = t;
    prevTick = gpioTick();
    gpioSetWatchdog(pinX, timeout);
    gpioSetAlertFuncEx(pinX, _cb, dispenser);
}

void cancelTrigger(int pinX) {
    gpioSetAlertFunc(pinX, 0);
}