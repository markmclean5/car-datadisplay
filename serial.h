// This is start of the header guard.  SERIAL_H can be any unique name.  By convention, we use the name of the header file.
#ifndef SERIALCLASS_H
#define SERIALCLASS_H

#include <termios.h>		//Used for UART
using namespace std;


class Serial {
private:

	int uart_filestream;
	string serialDevice;
	uint64_t readTimeoutTime;
	uint