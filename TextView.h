#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include "TouchableObject.h"

/********************************/
/*		TextView Class			*/
/********************************/

using namespace std;

class TextView : public TouchableObject {
private:
	string textViewIdentifier;
	int width, height;
	int borderWidth;
	int centerX, centerY;
	int bottomLeftX, bottomLeftY;
	int rectWidth, rectHeight;
	int cornerRadius;
	int fontSize;
	int textPadding;
	int numLines;
	string* textViewData;

	float borderColor[4];
	float borderColorAlpha;
	float backgroundColor[4];
	float backgroundColorAlpha;
	float textColor[4];
	float textColorAlpha;
	int currentLine;

public:
	TextView(int, int, int, int, string);		// TextView Constructor: center X, center Y, width, height, identifier
	void configure(string);
	void update();
	void addNewLine(string);
	void clearLastLine(void);
	void clear(void);
};

#endif
