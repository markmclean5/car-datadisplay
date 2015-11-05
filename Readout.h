#ifndef BUTTON_H
#define BUTTON_H

#include "TouchableObject.h"

/********************************/
/*		Button Class			*/
/********************************/
using namespace std;

class Button : public TouchableObject
{
private:
	// Timing related properties
	int desiredRefreshRate;
	uint64_t lastUpdateTime;

	// Graphics related properties
	float* valueColor;
	float* labelColor;
	float* backgroundColor;
	float* borderColor;
	int borderWidth;
	int fontSize;
	string label;
	char* formatSpecifier;
	int numLines;
	char labelAlign;
	char valueAlign;
	int decPlaces;

	// Size & location related properties
	int readoutWidth, readoutHeight;		// Size of entire readout, including border stroke
	int rectWidth, rectHeight;				// Size of rectangle to be drawn
	int centerX, centerY;					// Location of readout center
	int bottomLeftX, bottomLeftY;			// Location of rectangle bottom left corner

public:
	Readout(int, int, int, int, int);		// Readout constructor: center X, center Y, width, height
	void setBackgroundColor(float*);		
	void setBorder(float*, int);			// Set border color, border width

	// Button Customization
	enableValue(char, char);				// enables value (vertical alignment (T,C.B), horizontal (L,C,R))
	enableText(char, char);					// enables value (vertical alignment (T,C.B), horizontal (L,C,R))
	setValueColor(float*);					// set value color
	setLabelColor(float*);					// set label color
	void setValueDecPlaces(int);			// Set value number of decimal places
	void setText(string);
	void setValue(float); 
	void setValueRefreshRate(int);			// Set desired refresh frequency (Hz) 
	void draw(void);						// Draw readout

	// ** figure out how to save configuration of object in text file to be read @ runtime
	// implement object identifier strings?
};

#endif