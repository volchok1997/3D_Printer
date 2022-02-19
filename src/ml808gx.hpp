#ifndef __ML808GX_HPP__
#define __ML808GX_HPP__

#include <string>

class ML808GX {
private:
    int dispenser_status;               // 1: dispensing, 0: idle    
    int com_fd;
    const int COMM_DELAY=10;
    const int EOT_WAIT_US = 1000;
    const int ACK_WAIT_US = 10000;
    const int MSG_WAIT_US = 1000;
    // Pre-defined communication command
    const std::string STX = "\x02";
	const std::string ETX = "\x03";
	const std::string ENQ = "\x05";
	const std::string ACK = "\x06";
	const std::string A0  = STX+"02A02D"+ETX;           // "\x02\x30\x32\x41\x30\x32\x44\x03"
	const std::string A2  = STX+"02A22B"+ETX;           // "\x02\x30\x32\x41\x32\x32\x42\x03"
	const std::string EOT = "\x04";
	const std::string CAN = STX+"021835"+ETX;         //"\x02""0418186C\x03";
    // Command
    const std::string CMD_F_RM = STX+"05RM   9C"+ETX;
    const std::string CMD_F_DI = STX+"04DI  CF"+ETX;
private:
    int CmdInit();
    int CmdEndWithData(char buf[], int size);
    int CmdEnd();
    
public:
    ML808GX();

    int ConnectSerial(const char* dev, int baudrate);
    int VerifyDispenser();
    int GetDispenserStatus();
    int StartDispense();
    int StopDispense();
    int ToggleDispense();
};

#endif
