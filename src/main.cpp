#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <sys/stat.h> //libraries to open(USB)
#include <fcntl.h>

#include <termios.h>
#include <unistd.h>

#include <iostream>
using namespace std;

#include "INIReader.h"

//#include "yocto_api.h"
//#include "yocto_pwminput.h"

// default configuration
const char* DEFAULT_CONFIG_FILE = "config.ini";
const char* DEFAULT_LOG_FILE = "workdata.log";
const char* DEFAULT_ML808GX_SERIAL_PORT = "COM1";
const char* DEFAULT_MICROPLOTTER_SIG_READER_USB_PORT = "USB10";

const char* STX = "\x02";
const char*	ETX = "\x03";

void system_sig_handler(int s) {
    exit(s);
}

int main(int argc, char *argv[]) {
    
    
    int flags, opt;

    char logFile[256];
    char cfgFile[256];

    char comPort[256];
    char usbPort[256];

    // default setup
    sprintf(cfgFile, "%s",DEFAULT_CONFIG_FILE);
    sprintf(logFile, "%s",DEFAULT_LOG_FILE);

    sprintf(comPort, "%s", DEFAULT_ML808GX_SERIAL_PORT);
    sprintf(usbPort, "%s", DEFAULT_MICROPLOTTER_SIG_READER_USB_PORT);
    
    // try to load config file to overwrite default value
    INIReader* reader = new INIReader(cfgFile);
    if(reader->ParseError() == 0) {
        sprintf(logFile, "%s", reader->Get("SYSTEM", "LOG", DEFAULT_LOG_FILE).c_str());

        sprintf(comPort, "%s", reader->Get("ML808GX", "PORT", DEFAULT_ML808GX_SERIAL_PORT).c_str());
        sprintf(usbPort, "%s", reader->Get("MICROPLOTTER_SIG_DETECTOR", "PORT", DEFAULT_MICROPLOTTER_SIG_READER_USB_PORT).c_str());
    }
    delete reader;

    while((opt = getopt(argc, argv, "c:p:u:l:")) != -1) {
        switch (opt) {
            case 'c':
                sprintf(cfgFile, "%s", optarg);
                reader = new INIReader(cfgFile);
                if(reader->ParseError() == 0) {
                    sprintf(logFile, "%s", reader->Get("SYSTEM", "LOG", DEFAULT_LOG_FILE).c_str());

                    sprintf(comPort, "%s", reader->Get("ML808GX", "PORT", DEFAULT_ML808GX_SERIAL_PORT).c_str());
                    sprintf(usbPort, "%s", reader->Get("MICROPLOTTER_SIG_DETECTOR", "PORT", DEFAULT_MICROPLOTTER_SIG_READER_USB_PORT).c_str());
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
                    fprintf(stderr, "USB port format error.\n");
                    exit(EXIT_FAILURE);
                }
                sprintf(usbPort, "%s", optarg);
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-c CONFIG_FILE] [-p COM_PORT] [-u USB_PORT] [-l LOG_FILE]\n",
                            argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    fprintf(stderr, "ML808GX control port: %s\n"
                    "Microplotter Signal detector port: %s\n\n", comPort, usbPort);
    
    // TODO: Check ports, validate equipments
    std::cout << "Opening USB port." << std::endl;
    /* Open File Descriptor */
    int USB = open( "/dev/ttyUSB0", O_RDWR| O_NOCTTY );
    
    //Open USB error handiling
    if (USB < 0) {
        cout << "Error while opening device... " << "errno = " << errno << endl;
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

    /* Setting other Port Stuff */
    tty.c_cflag     &=  ~PARENB;            // Make 8n1
    tty.c_cflag     &=  ~CSTOPB;
    tty.c_cflag     &=  ~CSIZE;
    tty.c_cflag     |=  CS8;

    tty.c_cflag     &=  ~CRTSCTS;           // no flow control
    tty.c_cc[VMIN]   =  1;                  // read doesn't block
    tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
    tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

    /* Make raw */
    cfmakeraw(&tty);

    /* Flush Port, then applies attributes */
    tcflush( USB, TCIFLUSH );
    if ( tcsetattr ( USB, TCSANOW, &tty ) != 0) {
    std::cout << "Error " << errno << " from tcsetattr" << std::endl;
    }

    //write
    //dispence \x0204DI__CF\x03
    //ROM version \x0205RM___9C\x03
    //underscore "_" may have to be changed to \x20
    std::cout << "Writing \\x0204DI__CF\\x03." << std::endl;
    unsigned char cmd[] = "\\x0204DI__CF\\x03";
    int n_written = 0,
        spot = 0;

    do {
        n_written = write( USB, &cmd[spot], 1 );
        spot += n_written;
    } while (cmd[spot-1] != '\r' && n_written > 0);

    //read
    int n = 0;
    char buf = '\0';

    /* Whole response*/
    char response[1024];
    memset(response, '\0', sizeof response);

    do {
        n = read( USB, &buf, 1 );
        sprintf( &response[spot], "%c", buf );
        spot += n;
    } while( buf != '\r' && n > 0);

    if (n < 0) {
        std::cout << "Error reading: " << strerror(errno) << std::endl;
    }
    else if (n == 0) {
        std::cout << "Read nothing!" << std::endl;
    }
    else {
        std::cout << "Response: " << response << std::endl;
    }

    signal(SIGINT, system_sig_handler);

    // TODO: tracking signal changes and control the dispenser
    while(1) {

    }

    exit(EXIT_SUCCESS);
}
