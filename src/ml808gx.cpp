
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
//#include <string.h>
#include <fcntl.h>
#include "serial.h"

#include <iostream>
#include <string>
//using std::string;

#include "ml808gx.hpp"

ML808GX::ML808GX() {
    dispenser_status = 0;
    com_fd = -1;
}

int ML808GX::ConnectSerial(const char* dev, int baudrate) {
    int err;
    com_fd = open(dev, O_RDWR | O_NOCTTY);
    if(com_fd<0) {
        fprintf (stderr, "error %d opening %s: %s", errno, dev, strerror (errno));
        return com_fd;
    }

    err = set_interface_attribs (com_fd, B(baudrate), 0);  // set speed to 115,200 bps, 8n1 (no parity)
    if(err == -1)
        return err;
    set_blocking (com_fd, 0);

    return 0;
}

int ML808GX::CmdInit() {
    char buf [16];
    int n;
    int c;
    int n_written = 0;
    // send ENQ
    n_written = write(com_fd, ENQ.c_str(), ENQ.size());
    // wait ACK
    // FIXME: test the delay is OK
    c = 0;
    buf[0] = 0;
    while((n = read(com_fd, buf, 8)) == 0) {
	usleep (COMM_DELAY); 
        c+= COMM_DELAY;
        if(c>ACK_WAIT_US)
            break;
    }

    if(buf[0] == ACK.c_str()[0]) {
        fprintf(stderr, "ACK received in %d us\n", c);
        return 0;
    } else {
        fprintf(stderr, "ACK timeout in %d us\n", ACK_WAIT_US);
        return -1;
    }
}

int ML808GX::CmdEndWithData(char data[], int size) {
    char buf[64] = {};
    int n=0;
    int c=0;
    int n_written = 0;
    int data_size = 3;
    while((n += read(com_fd, buf+n, 64-n)) < 8) {
        usleep (COMM_DELAY); 
	std::cout << "n = " << n << std::hex << ", read char: 0x" << (0xFF & buf[0]) << std::endl;
        c+= COMM_DELAY;
        if(c>ACK_WAIT_US)
            break;
    }
    std::cout<<"DEBUG:"<<buf<<std::endl;
    buf[n] = 0;
    n=0;
    std::string reply(buf);
    if(reply == A0) {
        // send ACK
        n_written = write(com_fd, ACK.c_str(), ACK.size());
        c=0;
        // read message
        while((n += read(com_fd, data+n, size)) <size) {
            usleep (COMM_DELAY); 
            c+= COMM_DELAY;
            if(c>MSG_WAIT_US)
                break;
            if(n>=3) {
                if(data[1]<='9')
                    size = (data[1]-'0')<<4;
                else
                    size = (data[1]-'A'+10)<<4;
                if(data[2]<='9')
                    size += (data[2]-'0');
                else
                    size += (data[2]-'A'+10);
                size += 3+3;
            }
        }
        data[n]=0;
        // send EOT
        n_written = write(com_fd, EOT.c_str(), EOT.size());
        return n;
    } else if(reply == A2) {
        fprintf(stderr, "Received A2, message failed\n");
        n_written = write(com_fd, CAN.c_str(), CAN.size());
        usleep(ACK_WAIT_US);
        return -1;
    } else {
        fprintf(stderr, "CmdEndWithData error, DEBUG info: size%d, %s\n", n, buf);
        return -1;
    }
}

int ML808GX::CmdEnd() {
    char buf[16] = {};
    int n=0;
    int c=0;
    int n_written = 0;

    while((n += read(com_fd, buf+n, 16-n)) <8) {
	std::cout << "n = " << n << std::hex << ", read char: 0x" << (buf) << std::endl;
        usleep (COMM_DELAY); 
        c+= COMM_DELAY;
        if(c>ACK_WAIT_US)
            break;
    }
    buf[n]=0;
    std::string reply(buf);
    if(reply == A0) {
        // send EOT
        fprintf(stderr, "Received A0, Done\n");
        n_written = write(com_fd, EOT.c_str(), EOT.size());
        usleep(ACK_WAIT_US);
        return 0;
    } else if(reply == A2) {
        fprintf(stderr, "Received A2, message failed\n");
        n_written = write(com_fd, CAN.c_str(), CAN.size());
        usleep(ACK_WAIT_US);
        return -1;
    } else {
        fprintf(stderr, "CmdEnd error, DEBUG info: size%d, %s\n", n, buf);
        return -1;
    }
}

int ML808GX::VerifyDispenser() {
    int err;
    char buf[64] = {};
    if(com_fd<0)
        return com_fd;      // com not connected
    
    // Prepare send
    err = CmdInit();
    if(err<0) {
        fprintf(stderr, "Cmd ACK error..\n");
        return err;
    }
    // Send command
    write (com_fd, CMD_F_RM.c_str(), CMD_F_RM.size());           // send RM command

    int sz = CmdEndWithData(buf, sizeof(buf));
    if(sz>0) {
        fprintf(stderr, "Got ROM version: %s\n", buf);
    } else {
        fprintf(stderr, "Read ROM version error\n");
    }
    return 0;
}

int ML808GX::ToggleDispense() {
    int err;
    // Prepare send
    fprintf(stderr, "ToggleDispense ...\n");
    err = CmdInit();
    if(err<0) {
        fprintf(stderr, "Cmd ACK error..\n");
        return err;
    }
    // Send command
    write (com_fd, CMD_F_DI.c_str(), CMD_F_DI.size());           // send RM command

    err = CmdEnd();
    if(err==0) {
        fprintf(stderr, "Toggle dispense Done\n");
    } else {
        fprintf(stderr, "Toggle dispense Error\n");
    }
    return err;
}

int ML808GX::StartDispense() {
    int err;
    if(dispenser_status==1)   {// already started
        return -1;
    }

    err = ToggleDispense();
    if(err==0) {
        dispenser_status = 1;
    }
    
    return err;
}

int ML808GX::StopDispense() {
    int err;
    if(dispenser_status==0)  // already stopped
        return -1;
    
    err = ToggleDispense();
    if(err==0) {
        dispenser_status = 0;
    }
    
    return err;
}

int ML808GX::GetDispenserStatus() {
    return dispenser_status;
}
