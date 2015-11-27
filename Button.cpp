/********************************/
/*		Button Class			*/
/********************************/
#include <iostream>
#include "VG/openvg.h"		//
#include "VG/vgu.h"			//
#include "fontinfo.h"		// OpenVG
#include "shapes.h"			//
#include <cmath>			// Math (float remainder operation)
#include "Button.h"
#include <stdio.h>
#include <bcm2835.h>

using namespace std;		// ??

string valueString = "";

/* Readout Constructor */
Button::Button(int cx, int cy, int w, int h)
{
	lastUpdateTime = 0;
	centerX = cx;
	centerY = cy;
	readoutWidth = w;
	readoutHeight = h;
	valueColor = new float[4]{0,1,0,1};
	valueColorAlpha = valueColor[3];
	textColor = new float[4]{0,1,0,1};
	textColorAlpha = textColor[3];
	backgroundColor = new float[4]{0,0,0,0.6};
	backgroundColorAlpha = backgroundColor[3];
	borderColor = new float[4]{0,1,0,1};
	borderColorAlpha = borderColor[3];
	containsText = false;
	containsValue = false;
	cornerRadius = 0;

	setRectangular();
	setRectWidthHeight(readoutWidth, readoutHeight);	// Called by derived class to set rectangular touch area size
	setRectCenter(centerX, centerY);	// Called by derived class to set rectangular touch area bottom left corner

}
/* Button draw */
void Button::draw(void)
{
	update();
}

/* Button update */
void Button::update(void)
{
	// Handle movement: current position is not desired position
	if(centerX != getDesiredPosX() || centerY != getDesiredPosY())
	{
		centerX = getDesiredPosX();
		centerY = getDesiredPosY();
		bottomLeftX = centerX - (rectWidth+borderWidth/2) / 2;
		bottomLeftY = centerY - (rectHeight+borderWidth/2) / 2;
	}

	// Handle fade:
	if(getDesiredFadePercentage() != 0) {
		float alphaScalar = (100. - getDesiredFadePercentage()) / 100.;
		backgroundColor[3] = backgroundColorAlpha * alphaScalar;
		borderColor[3] = borderColorAlpha * alphaScalar;
		textColor[3] = textColorAlpha * alphaScalar;
		valueColor[3] = valueColorAlpha * alphaScalar;
	}

	setfill(backgroundColor);
	StrokeWidth(borderWidth);
	setstroke(borderColor);
	if(cornerRadius == 0) Rect(bottomLeftX, bottomLeftY, rectWidth, rectHeight);
	else
	{
		float cornerHeight = 0.01 * cornerRadius * rectWidth;
		Roundrect(bottomLeftX, bottomLeftY, rectWidth, rectHeight, cornerRadius, cornerRadius);
	}

	if(containsText)
	{
			setfill(textColor);
			StrokeWidth(0);
			textFontSize = (rectHeight-borderWidth)/2;
			int textWidth = TextWidth((char*)text.c_str(), SansTypeface, textFontSize);

			while(textWidth > 0.9*rectWidth)
			{
				textFontSize--;
				textWidth = TextWidth((char*)text.c_str(), SansTypeface, textFontSize);
			}
			if(textVertAlign == 'T')
				TextMid(centerX, bottomLeftY+rectHeight-textFontSize, (char*)text.c_str(), SansTypeface, textFontSize-2);
			if(textVertAlign == 'C')
				TextMid(centerX, centerY-textFontSize/2, (char*)text.c_str(), SansTypeface, textFontSize-2);
			if(textVertAlign == 'B')
				TextMid(centerX, bottomLeftY+borderWidth, (char*)text.c_str(), SansTypeface, textFontSize-2);
	}
	if(containsValue)
	{
		setfill(valueColor);
		if(valueVertAlign == 'T')
			TextMid(centerX, bottomLeftY+rectHeight-valueFontSize, (char*)valueString.c_str(), SansTypeface, valueFontSize-2);
		if(valueVertAlign == 'C')
			TextMid(centerX, centerY-valueFontSize/2, (char*)valueString.c_str(), SansTypeface, valueFontSize-2);
		if(valueVertAlign == 'B')
			TextMid(centerX, bottomLeftY+borderWidth, (char*)valueString.c_str(), SansTypeface, valueFontSize-2);
	}
}


/*
void Button::selectProcessing(void)
{

}

*/

/* Button setters */

/*
void Button::enableSelect()
{
	selectEnabled = true;
}
void Button::disableSelect(void)
{
	selectEnabled = true;
}
void Button::setSelectedBackgroundColor(float color[4])
{
	selectedBackgroundColor[0] = color[0];
	selectedBackgroundColor[1] = color[1];
	selectedBackgroundColor[2] = color[2];
	selectedBackgroundColor[3] = color[3];
}

void Button::setSelectedTextColor(float color[4])
{
	selectedTextColor[0] = color[0];
	selectedTextColor[1] = color[1];
	selectedTextColor[2] = color[2];
	selectedTextColor[3] = color[3];
}

void Button::enablePressColor(void)
{
	pressColorEnabled = true;
}
void Button::disablePressColor(void)
{
	pressColorEnabled = true;
}
void Button::setPressBackgroundColor(float color[4])
{
	pressedBackgroundColor[0] = color[0];
	pressedBackgroundColor[1] = color[1];
	pressedBackgroundColor[2] = color[2];
	pressedBackgroundColor[3] = color[3];
}

void Button::setPressTextColor(float color[4])
{
	pressedTextColor[0] = color[0];
	pressedTextColor[1] = color[1];
	pressedTextColor[2] = color[2];
	pressedTextColor[3] = color[3];
}
*/
void Button::setCornerRadius(int rad)
{
	cornerRadius = rad;
}

void Button::setValueDecPlaces(int dec)						// Set number of digits before & after decimal
{
	valueDecPlaces = dec;
	char decPlacesText[10];
	sprintf(decPlacesText, "%d", valueDecPlaces);
	formatSpecifierString = "%.";
	formatSpecifierString.append(decPlacesText);
	formatSpecifierString.append("f");
}
void Button::setBackgroundColor(float color[4])
{
	backgroundColor[0] = color[0];
	backgroundColor[1] = color[1];
	backgroundColor[2] = color[2];
	backgroundColor[3] = color[3];
	backgroundColorAlpha = backgroundColor[3];
}
void Button::setValueColor(float color[4])
{
	valueColor[0] = color[0];
	valueColor[1] = color[1];
	valueColor[2] = color[2];
	valueColor[3] = color[3];
	valueColorAlpha = valueColor[3];
}
void Button::setTextColor(float color[4])
{
	textColor[0] = color[0];
	textColor[1] = color[1];
	textColor[2] = color[2];
	textColor[3] = color[3];
	textColorAlpha = textColor[3];
}
void Button::setBorder(float color[4], int width)		// Set border color, border width
{
	borderWidth = width;
	borderColor[0] = color[0];
	borderColor[1] = color[1];
	borderColor[2] = color[2];
	borderColor[3] = color[3];
	borderColorAlpha = borderColor[3];
	rectHeight = readoutHeight-borderWidth;
	rectWidth = readoutWidth - borderWidth;
	bottomLeftX = centerX - (rectWidth+borderWidth/2) / 2;
	bottomLeftY = centerY - (rectHeight+borderWidth/2) / 2;
}
void Button::setValueRefreshRate(int rate)		// Set desired refresh frequency (Hz)
{
	desiredRefreshRate = rate;
}
void Button::enableValue(char v)
{
	valueVertAlign = v;
	containsValue = true;
}
void Button::enableText(char v)
{
	textVertAlign = v;
	containsText = true;
}
void Button::setText(string txt)						// Set readout label
{
	text.assign(txt);
}
void Button::setValue(float val)						// Set readout label
{
	value = val;
	uint64_t currentTime = bcm2835_st_read();
	if(desiredRefreshRate == 0)	desiredRefreshRate = 5;
	uint64_t nextTime = lastUpdateTime + (1000000/desiredRefreshRate);
	if(containsValue && currentTime>=nextTime)
	{
		setfill(valueColor);
		StrokeWidth(0);
		valueFontSize = (rectHeight-borderWidth)/2;
		char valueText[10];
		sprintf(valueText, (char*)formatSpecifierString.c_str(), value);
		valueString=valueText;
		lastUpdateTime = currentTime;
	}
	int valueWidth = TextWidth((char*)valueString.c_str(), SansTypeface, valueFontSize);
	while(valueWidth > 0.9*rectWidth)
	{
		valueFontSize--;
		valueWidth = TextWidth((char*)valueString.c_str(), SansTypeface, valueFontSize);
	}
	
}
