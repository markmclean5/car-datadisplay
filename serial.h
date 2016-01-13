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
	uint64_t lastWriteTime;
	int readTimeout;

	string readUntilBuffer;
	string writeBuffer;
	bool endCharReceived;
	char endChar;
	bool timeoutState;

public:
	Serial(string, speed_t);		// Serial Constructor: serial device name (baud rate not yet implemented)
	
	void setReadTimeout(int);	// Set serial read timeout (reached if no data / or end character mnot received)
	void setEndChar(char);		// Set end character for serialReadUntil

	int getResponseTime(void);	// Get time beween send and receive
	bool timedOut(void);		// Checks if timeout interval was reached

	string serialRead(void);	
	string serialReadUntil(void);
	void serialWrite(string);


};

#endif