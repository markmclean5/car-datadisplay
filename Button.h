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

	string buttonIdentifier;
	bool givenSizeAndLocation;

	// Size & location related properties
	int readoutWidth, readoutHeight;		// Size of entire readout, including border stroke
	int rectWidth, rectHeight;				// Size of rectangle to be drawn
	int centerX, centerY;					// Location of readout center
	int bottomLeftX, bottomLeftY;			// Location of rectangle bottom left corner
	int cornerRadius;

	// Timing related properties
	int desiredRefreshRate;
	uint64_t lastUpdateTime;

	// Graphics related properties
	float backgroundColor[4];
	float backgroundColorAlpha;
	float borderColor[4];
	float borderColorAlpha;
	int borderWidth;

	// Text properties
	float* textColor;
	float textColorAlpha;
	int textFontSize;
	char textVertAlign;
	string text;
	bool containsText;

	// Value properties
	float* valueColor;
	float valueColorAlpha;
	int valueFontSize;	
	char valueVertAlign;
	string formatSpecifierString;
	int valueDecPlaces;
	float value;
	bool containsValue;


	// Selection Properties
	bool selectEnabled;
	bool selected;
	float* selectedBackgroundColor;
	float* selectedTextColor;

	// Pressed Color Properties
	bool pressedColorEnabled;
	float* pressedBackgroundColor;
	float* presssedTextColor;
	void configure(string);

public:
	Button(string);							// Button constructor: load size and location from config file
	Button(int, int, int, int, string);		// Button constructor: takes size and location, config string
	
	void setBackgroundColor(float*);		// Set background color
	void setBorder(float*, int);			// Set border color, border width
	void setCornerRadius(int);				// Set corner radius if rounded 

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
	string getIdentifier(void);
};

#endif