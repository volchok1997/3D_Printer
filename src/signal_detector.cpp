#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <chrono>
#include <ctime>


#include <pigpio.h>

#include "signal_detector.hpp"
#include "ml808gx.hpp"

#include "gpio_direct_register_access.hpp"

const int DBG_GPIO=21;


int signalDetectorInitial(int gpioX) {
    if(gpioInitialise()<0)
        return -1;
    setup_io();

    INP_GPIO(gpioX); // must use INP_GPIO before we can use OUT_GPIO

    // DEBUG signal
    INP_GPIO(DBG_GPIO);  
    OUT_GPIO(DBG_GPIO);

    return 0;
}

int trackWave(int gpioX, int t, void* user){
    ML808GX* dispenser = (ML808GX*)user;
    int prev_tick = gpioTick();
    int pinState = 0;
    unsigned debounce=t;
    unsigned start;
    unsigned end;
    int max=0;

    auto timeStart= std::chrono::system_clock::now();
    auto timeEnd  = std::chrono::system_clock::now();
    std::time_t currTime = std::chrono::system_clock::to_time_t(timeEnd);
    std::chrono::duration<double> elapsed_seconds = timeEnd-timeStart;

    while(1) {
        //int pin = gpioRead(pinX);
        int pin = (GET_GPIO(gpioX) !=0);
        auto tick = gpioTick();
	
        //if no change wait delay and check again. reset start tick for HIGH
        if(pinState == pin) {
            if(pin == 1)
                start = tick;
            gpioDelay(1);
            continue;
        }
    
        //if rising edge detected start counting
        if(pin == 1) {
            start = tick;
            if(pinState == 0) {
                dispenser->StartDispense();
                GPIO_SET(DBG_GPIO);
                timeStart= std::chrono::system_clock::now();
                currTime = std::chrono::system_clock::to_time_t(timeStart);
                fprintf(stderr, "%s : Start dispenser\n", std::ctime(&currTime));
                pinState = pin;
            }
        //if LOW then check the timeout
        }else {
            end = tick;
            if(debounce < end-start){
                dispenser->StopDispense();
                GPIO_CLR(DBG_GPIO);
                timeEnd= std::chrono::system_clock::now();
                currTime = std::chrono::system_clock::to_time_t(timeEnd);
                elapsed_seconds = timeEnd-timeStart;
                fprintf(stderr, "%s : Stop dispenser, duration %f seconds\n", std::ctime(&currTime), elapsed_seconds.count());
                fprintf(stderr, "range detected : %d us\n", end-start);
                max = end-start;
                pinState = pin;
            }
        }
        gpioDelay(1);
    }

    return 0;
}
