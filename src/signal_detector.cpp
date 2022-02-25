#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include <pigpio.h>

#include "signal_detector.hpp"
#include "ml808gx.hpp"

//
//  How to access GPIO registers from C-code on the Raspberry-Pi
//  Example program
//  15-January-2012
//  Dom and Gert
//  Revised: 15-Feb-2013


// Access from ARM Running Linux

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int  mem_fd;
void *gpio_map;

// I/O access
volatile unsigned *gpio;


// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH

#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock

void setup_io();


//
// Set up a memory regions to access GPIO
//
void setup_io()
{
   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }

   /* mmap GPIO */
   gpio_map = mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      GPIO_BASE         //Offset to GPIO peripheral
   );

   close(mem_fd); //No need to keep mem_fd open after mmap

   if (gpio_map == MAP_FAILED) {
      printf("mmap error %p\n", gpio_map);//errno also set!
      exit(-1);
   }

   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;


} // setup_io




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
