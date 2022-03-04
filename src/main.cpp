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

#include "ml808gx.hpp"
#include "signal_detector.hpp"
#include "gpio_direct_register_access.hpp"


// default configuration
const char* DEFAULT_CONFIG_FILE = "config.ini";
const char* DEFAULT_LOG_FILE = "workdata.log";
const char* DEFAULT_ML808GX_SERIAL_PORT = "COM1";
const int DEFAULT_RPI_PWM_READ_PIN = 6;
const int DEFAULT_COM_BAUDRATE = 115200;


void system_sig_handler(int s) {
    exit(s);
}

int main(int argc, char *argv[]) {
    int flags, opt;

    char logFile[256];
    char cfgFile[256];

    char comPort[256];
    int baudrate;
    int rpiPwmPin;

    ML808GX dispenser;

    // default setup
    sprintf(cfgFile, "%s",DEFAULT_CONFIG_FILE);
    sprintf(logFile, "%s",DEFAULT_LOG_FILE);

    sprintf(comPort, "%s", DEFAULT_ML808GX_SERIAL_PORT);
    rpiPwmPin = DEFAULT_RPI_PWM_READ_PIN;
    baudrate = DEFAULT_COM_BAUDRATE;
    
    // try to load config file to overwrite default value
    INIReader* reader = new INIReader(cfgFile);
    if(reader->ParseError() == 0) {
        sprintf(logFile, "%s", reader->Get("SYSTEM", "LOG", DEFAULT_LOG_FILE).c_str());
        sprintf(comPort, "%s", reader->Get("ML808GX", "PORT", DEFAULT_ML808GX_SERIAL_PORT).c_str());
        baudrate = reader->GetInteger("ML808GX", "BAUDRATE", DEFAULT_COM_BAUDRATE);
        rpiPwmPin = reader->GetInteger("MICROPLOTTER_SIG_DETECTOR", "PIN", DEFAULT_RPI_PWM_READ_PIN);
    }
    delete reader;

    while((opt = getopt(argc, argv, "c:p:i:l:r:")) != -1) {
        switch (opt) {
            case 'c':
                sprintf(cfgFile, "%s", optarg);
                reader = new INIReader(cfgFile);
                if(reader->ParseError() == 0) {
                    sprintf(logFile, "%s", reader->Get("SYSTEM", "LOG", DEFAULT_LOG_FILE).c_str());
                    sprintf(comPort, "%s", reader->Get("ML808GX", "PORT", DEFAULT_ML808GX_SERIAL_PORT).c_str());
                    baudrate = reader->GetInteger("ML808GX", "BAUDRATE", DEFAULT_COM_BAUDRATE);
                    rpiPwmPin = reader->GetInteger("MICROPLOTTER_SIG_DETECTOR", "PIN", DEFAULT_RPI_PWM_READ_PIN);
                    break;
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
            case 'i':
                if(optarg == NULL) {
                    fprintf(stderr, "RPI pin format error\n");
                    exit(EXIT_FAILURE);
                }
                rpiPwmPin = atoi(optarg);
                break;
            case 'r':
                if(optarg == NULL) {
                    fprintf(stderr, "COM rate error.\n");
                    exit(EXIT_FAILURE);
                }
                baudrate =atoi(optarg);
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-c CONFIG_FILE] [-p COM_PORT] [-i RPI_pwm_read_pin] [-l LOG_FILE]\n",
                            argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    fprintf(stderr, "ML808GX control port: %s, rate: %d\n"
                    "Raspberry Signal detector PIN: %d\n\n", comPort, baudrate, rpiPwmPin);
    
    fprintf(stderr, "Please verify: Sonoplot Dispenser frequency=200KHz, Voltage=10V\n");
    
    int err = signalDetectorInitial(rpiPwmPin);
    fprintf(stderr, "initial RPI input pin %d, result: %d\n", rpiPwmPin, err);
    if(err<0)
        exit(1);

    // Verify dispenser
    dispenser.ConnectSerial(comPort, baudrate);
    dispenser.VerifyDispenser();

    signal(SIGINT, system_sig_handler);

    // main signal tracking loop
    trackingWave(rpiPwmPin, 150, &dispenser);  // 150 us


    exit(EXIT_SUCCESS);
}
