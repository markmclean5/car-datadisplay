#include <iostream>
#include <stdio.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <stdlib.h>
#include <algorithm>
#include "bcm2835.h"

#include "Serial.h"

using namespace std;

Serial::Serial(string portName, speed_t baudRate) {
	readUntilBuffer = "";
	endCharReceived = false;
	uart_filestream = -1;
	//OPEN THE UART
	//The flags (defined in fcntl.h):
	//	Access modes (use 1 of these):
	//		O_RDONLY - Open for reading only.
	//		O_RDWR - Open for reading and writing.
	//		O_WRONLY - Open for writing only.
	//
	//	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
	//											if there is no input immediately available (instead of blocking). Likewise, write requests can also return
	//											immediately with a failure status if the output can't be written immediately.
	//
	//	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
	uart_filestream = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
	if (uart_filestream == -1)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
	}

	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	struct termios options;
	tcgetattr(uart_filestream, &options);
	options.c_cflag = baudRate | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart_filestream, TCIFLUSH);
	tcsetattr(uart_filestream, TCSANOW, &options);
}

string Serial::serialRead(void) {
//----- CHECK FOR ANY RX BYTES -----
	string data = "";
	char rx_buffer[256];
	rx_buffer[0] = '\0';
	if (uart_filestream != -1) {
		// Read up to 255 characters from the port if they are there
		int rx_length = read(uart_filestream, (void*)rx_buffer, 255);		//Filestream, buffer to store in, number of bytes to read (max)
		if (rx_length < 0) {
			//An error occured (will occur if there are no bytes)
		}
		else if (rx_length == 0) {
			//No data waiting
		}
		else {
			//Bytes received
			rx_buffer[rx_length] = '\0';
			//cout << rx_length <<" bytes received."<<endl;
			data.append(rx_buffer);
			// Remove all carriage returns (ascii 13)
			data.erase (std::remove(data.begin(), data.end(), (char)13), data.end());
		}
	}
	return data;
}

void Serial::serialWrite(string data) {
	data.append("\r");
	if (uart_filestream != -1) {
		int count = write(uart_filestream, data.c_str(), data.size());		//Filestream, bytes to write, number of bytes to write
		if (count < 0) {
			printf("UART TX error\n");
		}
	}
	lastWriteTime = bcm2835_st_read();
	readTimeoutTime = lastWriteTime + 1000*readTimeout;
	timeoutState = false;
	//endCharReceived = false;
	//readUntilBuffer = "";
}

string Serial::serialReadUntil(void) {
	string output = "";
	//uint64_t currentTime = bcm2835_st_read();
	//if(!endCharReceived && (currentTime >= readTimeoutTime)) {
	//	timeoutState = true;
	//	readUntilBuffer = "";
	//}
	string data = serialRead();
		if(!data.empty()) {
		readUntilBuffer.append(data);
	}
	size_t endCharLocation = readUntilBuffer.find_first_of(endChar);
	if(endCharLocation != std::string::npos){
		endCharReceived = true;
	}

	if(endCharReceived) {
		output.assign(readUntilBuffer.substr(0,endCharLocation+1));
		string temp = readUntilBuffer.substr(endCharLocation+1, std::string::npos);
		readUntilBuffer.assign(temp);
		endCharReceived = false;
	}
	return output;
}

bool Serial::timedOut(void) {
	return timeoutState;
}

void Serial::setEndChar(char c) {
	endChar = c;
}

void Serial::setReadTimeout(int timeout) {
	readTimeout = timeout;
}