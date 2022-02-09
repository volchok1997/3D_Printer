#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <sys/stat.h> //libraries to open(USB)
#include <fcntl.h>

#include <termios.h>
#include <unistd.h>

#include <iomanip>
#include <iostream>

#include <chrono>
#include <ctime> 


#include "INIReader.hpp"

//#include "yocto_api.h"
//#include "yocto_pwminput.h"

#include "ml808gx.hpp"
#include "yocto_pwm.hpp"

// default configuration
const char* DEFAULT_CONFIG_FILE = "config.ini";
const char* DEFAULT_LOG_FILE = "workdata.log";
const char* DEFAULT_ML808GX_SERIAL_PORT = "COM1";
const char* DEFAULT_MICROPLOTTER_SIG_READER_USB_PORT = "USB10";
const int DEFAULT_COM_BAUDRATE = 115200;


static ML808GX dispenser;
Yocto_PWM pwm;


static void pwmChangeCallback(YPwmInput *fct, const string &value) {
    int err;
    dispenser.ToggleDispense();
    auto n = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(n);

    if(dispenser.GetDispenserStatus()==0) {
        err = dispenser.StartDispense();
        std::cerr << std::ctime(&t) << "Start Dispenser, PWM:" << value<<", Err = "<<err<< std::endl;
    } else {
        err = dispenser.StopDispense();
        std::cerr << std::ctime(&t) << "Stop Dispenser, PWM:" << value<<", Err = "<<err<< std::endl;
    }
    
}


void system_sig_handler(int s) {
    exit(s);
}

int main(int argc, char *argv[]) {
    
    
    int flags, opt;

    char logFile[256];
    char cfgFile[256];

    char comPort[256];
    int baudrate;
    char usbPwmDevice[256];

    // default setup
    sprintf(cfgFile, "%s",DEFAULT_CONFIG_FILE);
    sprintf(logFile, "%s",DEFAULT_LOG_FILE);

    sprintf(comPort, "%s", DEFAULT_ML808GX_SERIAL_PORT);
    sprintf(usbPwmDevice, "%s", DEFAULT_MICROPLOTTER_SIG_READER_USB_PORT);
    baudrate = DEFAULT_COM_BAUDRATE;
    
    // try to load config file to overwrite default value
    INIReader* reader = new INIReader(cfgFile);
    if(reader->ParseError() == 0) {
        sprintf(logFile, "%s", reader->Get("SYSTEM", "LOG", DEFAULT_LOG_FILE).c_str());

        sprintf(comPort, "%s", reader->Get("ML808GX", "PORT", DEFAULT_ML808GX_SERIAL_PORT).c_str());
        baudrate = reader->GetInteger("ML808GX", "BAUDRATE", DEFAULT_COM_BAUDRATE);
        sprintf(usbPwmDevice, "%s", reader->Get("MICROPLOTTER_SIG_DETECTOR", "DEVICE", DEFAULT_MICROPLOTTER_SIG_READER_USB_PORT).c_str());
    }
    delete reader;

    while((opt = getopt(argc, argv, "c:p:u:l:r:")) != -1) {
        switch (opt) {
            case 'c':
                sprintf(cfgFile, "%s", optarg);
                reader = new INIReader(cfgFile);
                if(reader->ParseError() == 0) {
                    sprintf(logFile, "%s", reader->Get("SYSTEM", "LOG", DEFAULT_LOG_FILE).c_str());

                    sprintf(comPort, "%s", reader->Get("ML808GX", "PORT", DEFAULT_ML808GX_SERIAL_PORT).c_str());
                    sprintf(usbPwmDevice, "%s", reader->Get("MICROPLOTTER_SIG_DETECTOR", "PORT", DEFAULT_MICROPLOTTER_SIG_READER_USB_PORT).c_str());
                } else {
                    fprintf(stderr, "Config file error, check file existence or format\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                if(optarg == NULL) {
                    fprintf(stderr, "COM port format error.\n");
                    exit(EXIT_FAILURE);
                }
                sprintf(comPort, "%s", optarg);
                break;
            case 'l':
                if(optarg == NULL) {
                    fprintf(stderr, "Log file format error.\n");
                    exit(EXIT_FAILURE);
                }
                sprintf(logFile, "%s", optarg);
                break;
            case 'u':
                if(optarg == NULL) {
                    fprintf(stderr, "USB/Yocto-PWM device format error.\n");
                    exit(EXIT_FAILURE);
                }
                sprintf(usbPwmDevice, "%s", optarg);
                break;
            case 'r':
                if(optarg == NULL) {
                    fprintf(stderr, "COM rate error.\n");
                    exit(EXIT_FAILURE);
                }
                baudrate =atoi(optarg);
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-c CONFIG_FILE] [-p COM_PORT] [-u USB_PORT] [-l LOG_FILE]\n",
                            argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    fprintf(stderr, "ML808GX control port: %s, rate: %d\n"
                    "Microplotter Signal detector port: %s\n\n", comPort, baudrate, usbPwmDevice);
    
    // TODO: Check ports, validate equipments
    std::cout << "Opening USB port." << std::endl;
    /* Open File Descriptor */
    int USB = open( "/dev/ttyUSB0", O_RDWR| O_NOCTTY);
    
    //Open USB error handiling
    if (USB < 0) {
	std::cout << "Error while opening device... " << "errno = " << errno << std::endl;
        perror("Something went wrong with open()");
        exit(1);
    }
    
    struct termios tty;
    struct termios tty_old;
    memset (&tty, 0, sizeof tty);

    /* Error Handling */
    if ( tcgetattr ( USB, &tty ) != 0 ) {
    std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
    }

    /* Save old tty parameters */
    tty_old = tty;

    /* Set Baud Rate */
    cfsetospeed (&tty, (speed_t)B19200);
    cfsetispeed (&tty, (speed_t)B19200);


// test dispenser
    dispenser.ConnectSerial(comPort, baudrate);
    dispenser.VerifyDispenser();
    dispenser.StartDispense();
    sleep(10);
    dispenser.StopDispense();

// test pwm
//    yoctoTest();


    signal(SIGINT, system_sig_handler);

    // TODO: tracking signal changes and control the dispenser
    //while(1) {

    //}

// Don't touch this 
    //pwm.Detect();
    //pwm.EnablePWMDetection(pwmChangeCallback);
    //pwm.EnterEventMode();

    exit(EXIT_SUCCESS);
}
