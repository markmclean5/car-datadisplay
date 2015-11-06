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
	// Size & location related properties
	int readoutWidth, readoutHeight;		// Size of entire readout, including border stroke
	int rectWidth, rectHeight;				// Size of rectangle to be drawn
	int centerX, centerY;					// Location of readout center
	int bottomLeftX, bottomLeftY;			// Location of rectangle bottom left corner

	// Timing related properties
	int desiredRefreshRate;
	uint64_t lastUpdateTime;

	// Graphics related properties
	float* backgroundColor;
	float* borderColor;
	int borderWidth;

	// Text properties
	float* textColor;
	int textFontSize;
	char textVertAlign;
	string text;
	string lastText;
	bool containsText;

	// Value properties
	float* valueColor;
	int valueFontSize;	
	char valueVertAlign;
	char* valueFormatSpecifier;
	int valueDecPlaces;
	float value;
	bool containsValue;

public:
	Button(int, int, int, int);				// Button constructor: center X, center Y, width, height
	
	void setBackgroundColor(float*);		// Set background color
	void setBorder(float*, int);			// Set border color, border width

	void draw(void);
	void update(void);

	// Text methods
	void enableText(char);						// enables text, vertical alignment (T,C.B)
	void setTextColor(float*);				// set text color
	void setText(string);					// sets 

	// Value methods
	void enableValue(char);					// enables value (vertical alignment (T,C.B)
	void setValueColor(float*);					// set value color
	void setValueDecPlaces(int);			// Set value number of decimal places
	void setValue(float); 
	void setValueRefreshRate(int);			// Set desired refresh frequency (Hz) 
};

#endif