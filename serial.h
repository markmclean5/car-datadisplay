// This is start of the header guard.  SERIAL_H can be any unique name.  By convention, we use the name of the header file.
#ifndef SERIAL_H
#define SERIAL_H
 
// This is the content of the .h file, which is where the declarations go
int openSerial(void);
void readSerial(int, char*);

// This is the end of the header guard
#endif