#ifndef BUTTON_H
#define BUTTON_H

#include "TouchableObject.h"
#include "DisplayableObject.h"

/********************************/
/*		Button Class			*/
/********************************/
using namespace std;

class Button : public TouchableObject, public DisplayableObject {
private:

	VGImage bufferImage;
	bool bufferSaved;

	string buttonIdentifier;				// Button identifier string
	string buttonName;
	string name;
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

	// Button visual properties
	float backgroundColor[4];
	float backgroundColorAlpha;
	float selectedBackgroundColor[4];
	float selectedBackgroundColorAlpha;
	float borderColor[4];
	float borderColorAlpha;
	int borderWidth;
	float selectedBorderColor[4];
	float selectedBorderColorAlpha;
	int selectedBorderWidth;

	// Button text visual properties
	float* textColor;
	float textColorAlpha;
	float selectedTextColor[4];
	float selectedTextColorAlpha;
	float textFontSize;
	char textVertAlign;
	string text;
	bool containsText;

	// Value properties
	float* valueColor;
	float valueColorAlpha;
	float* selectedValueColor;
	float selectedValueColorAlpha;
	int valueFontSize;	
	char valueVertAlign;
	string formatSpecifierString;
	int valueDecPlaces;
	float value;
	bool containsValue;

	// Button selection properties
	bool selectable;
	bool selected;

	void configure(string);

public:
	Button(string);


	void update(touch_t) {}
	//~Button(void);						// Button destructor
	Button(int, int, int, int, string);		// Button constructor: takes size and location, config string
	Button(int, int, int, int);				// Button constructor: takes size and location, requires setter calls
	void setBackgroundColor(float*);		// Set background color
	void setSelectedBackgroundColor(float*);// Set selected background color
	void setBorder(float*, int);			// Set border color, border width
	void setSelectedBorder(float*, int);	// Set selected border color, border width
	void setCornerRadius(int);				// Set corner radius if rounded 
					
	void update(void); 						// Button update & draw function

	// Text methods
	void enableText(char);					// Enable text, vertical alignment (T,C.B)
	void setTextColor(float*);				// Set text color
	void setSelectedTextColor(float*);		// Set selected text color
	void setText(string);					// Set text 

	// Value methods
	void enableValue(char);					// Enable value (vertical alignment (T,C.B)
	void setValueColor(float*);				// Set value color
	void setSelectedValueColor(float*);		// Set selected value color
	void setValueDecPlaces(int);			// Set value number of decimal places
	void setValue(float); 
	void setValueRefreshRate(int);			// Set desired refresh frequency (Hz)

	// Name and Identifier methods 
	string getIdentifier(void);				// Get button identifier
	string getName(void);					// Get button name
	void setName(string);					// Set button name

	// Selection methods
	void select(void);
	void deselect(void);
	bool isSelected(void);
	void setSelectable(void);
};

#endif