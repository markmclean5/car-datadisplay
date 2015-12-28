// This is start of the header guard.  SERIAL_H can be any unique name.  By convention, we use the name of the header file.
#ifndef SERIALCLASS_H
#define SERIALCLASS_H

using namespace std;

class Serial {
private:

	int uart_filestream;
	string serialDevice;
	uint64_t lastReadTime;
	uint64_t lastWriteTime;
	int readTimeout;

	string readBuffer;
	string writeBuffer;

public:
	Serial(string, int);		// Serial Constructor: serial device name
	string serialRead(void);
	void serialWrite(string);

};

#endif