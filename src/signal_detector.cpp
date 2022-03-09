#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include <chrono>
#include <ctime>


//#include <pigpio.h>

#include "signal_detector.hpp"
#include "ml808gx.hpp"

#include "gpio_direct_register_access.hpp"

const int DBG_GPIO=21;

unsigned long sysTick() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    unsigned long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
    return time_in_micros;
}

int signalDetectorInitial(int gpioX) {
    int err;
    //if(gpioInitialise()<0)
    //    return -1;
    err = setup_io();
    if(err<0)
        return err;

    INP_GPIO(gpioX); // must use INP_GPIO before we can use OUT_GPIO

    // DEBUG signal
    INP_GPIO(DBG_GPIO);  
    OUT_GPIO(DBG_GPIO);

    return 0;
}

int trackWave(int gpioX, int t, void* user){
    ML808GX* dispenser = (ML808GX*)user;
    unsigned long prev_tick = sysTick(); //gpioTick();
    int pinState = 0;
    unsigned long debounce=t;
    unsigned long start;
    unsigned long end;
    unsigned long max=0;

    auto timeStart= std::chrono::system_clock::now();
    auto timeEnd  = std::chrono::system_clock::now();
    std::time_t currTime = std::chrono::system_clock::to_time_t(timeEnd);
    std::chrono::duration<double> elapsed_seconds = timeEnd-timeStart;

    while(1) {
        //int pin = gpioRead(pinX);
        int pin = (GET_GPIO(gpioX) !=0);
        auto tick = sysTick(); //gpioTick();
	
        //if no change wait delay and check again. reset start tick for HIGH
        if(pinState == pin) {
            if(pin == 1)
                start = tick;
            //gpioDelay(1);
        }else if(pin == 1) { //if rising edge detected start counting
            start = tick;
            if(pinState == 0) {
                dispenser->StartDispense();
                start = sysTick();
		GPIO_SET(DBG_GPIO);
                timeStart= std::chrono::system_clock::now();
                currTime = std::chrono::system_clock::to_time_t(timeStart);
                fprintf(stderr, "%s : Start dispenser\n", std::ctime(&currTime));
                pinState = pin;
            }
                                
        } else {  //if LOW then check the timeout
            end = tick;
            if(debounce < end-start){  
                dispenser->StopDispense();
                GPIO_CLR(DBG_GPIO);
                timeEnd= std::chrono::system_clock::now();
                currTime = std::chrono::system_clock::to_time_t(timeEnd);
                elapsed_seconds = timeEnd-timeStart;
                fprintf(stderr, "%s : Stop dispenser, duration %f seconds\n", std::ctime(&currTime), elapsed_seconds.count());
                max = end-start;
                pinState = pin;
            }
        }
        //gpioDelay(1);
        //usleep(1);
    }

    return 0;
}
