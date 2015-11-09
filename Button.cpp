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

/* Readout Constructor */
Button::Button(int cx, int cy, int w, int h)
{
	lastUpdateTime = 0;
	centerX = cx;
	centerY = cy;
	readoutWidth = w;
	readoutHeight = h;
	valueColor = new float[4]{0,1,0,1};
	textColor = new float[4]{0,1,0,1};
	backgroundColor = new float[4]{0,0,0,1};
	borderColor = new float[4]{0,1,0,1};
	lastText = "";
	containsText = false;
	containsValue = false;

	setRectangular();
	setRectWidthHeight(readoutWidth, readoutHeight);	// Called by derived class to set rectangular touch area size
	setRectCenter(centerX, centerY);	// Called by derived class to set rectangular touch area bottom left corner
}
/* Button draw */
void Button::draw(void)
{
	setfill(backgroundColor);
	StrokeWidth(borderWidth);
	setstroke(borderColor);
	Rect(bottomLeftX, bottomLeftY, rectWidth, rectHeight);
}

/* Button update */
void Button::update(void)
{
	uint64_t currentTime = bcm2835_st_read();
	if(desiredRefreshRate == 0)	desiredRefreshRate = 5;
	uint64_t nextTime = lastUpdateTime + (1000000/desiredRefreshRate);



	// Handle movement

	if(centerX != getDesiredPosX() || centerY != getDesiredPosY())
	{
		StrokeWidth(borderWidth+1);
		Stroke(0,0,0,1);
		Fill(0,0,0,1);
		Rect(bottomLeftX, bottomLeftY, rectWidth, rectHeight);

		centerX = getDesiredPosX();
		centerY = getDesiredPosY();

		bottomLeftX = centerX - (rectWidth+borderWidth) / 2;
		bottomLeftY = centerY - (rectHeight+borderWidth) / 2;
		setRectCenter(centerX, centerY);	// Called by derived class to set rectangular touch area bottom left corner
		draw();
		lastText = "";
<<<<<<< HEAD
=======

>>>>>>> origin/master
	}

	if(containsText)
	{
		if(text.compare(lastText)!=0)
		{
			lastText.assign(text);
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
			{
				setfill(backgroundColor);
				Rect(centerX-(0.9*rectWidth/2), bottomLeftY+rectHeight-textFontSize-1, 0.9*rectWidth, textFontSize-1);
				setfill(textColor);
				TextMid(centerX, bottomLeftY+rectHeight-textFontSize, (char*)text.c_str(), SansTypeface, textFontSize-2);
			}
			if(textVertAlign == 'C')
			{
				setfill(backgroundColor);
				Rect(centerX-(0.9*rectWidth/2), centerY-textFontSize/2-1, 0.9*rectWidth, textFontSize);
				setfill(textColor);
				TextMid(centerX, centerY-textFontSize/2, (char*)text.c_str(), SansTypeface, textFontSize-2);
			}
			if(textVertAlign == 'B')
			{	
				setfill(backgroundColor);
				Rect(centerX-(0.9*rectWidth/2), bottomLeftY+borderWidth-1, 0.9*rectWidth, textFontSize);
				setfill(textColor);
				TextMid(centerX, bottomLeftY+borderWidth, (char*)text.c_str(), SansTypeface, textFontSize-2);
			}
		}
	}
	if(containsValue && currentTime>=nextTime)
	{
			setfill(valueColor);
			StrokeWidth(0);
			valueFontSize = (rectHeight-borderWidth)/2;
			char valueText[10];
			sprintf(valueText, (char*)formatSpecifierString.c_str(), value);
			string  valueString(valueText);
			int valueWidth = TextWidth((char*)valueString.c_str(), SansTypeface, valueFontSize);

			while(valueWidth > 0.9*rectWidth)
			{
				valueFontSize--;
				valueWidth = TextWidth((char*)valueString.c_str(), SansTypeface, valueFontSize);
			}
			if(valueVertAlign == 'T')
			{
				setfill(backgroundColor);
				Rect(centerX-(0.9*rectWidth/2), bottomLeftY+rectHeight-valueFontSize-1, 0.9*rectWidth, valueFontSize-1);
				setfill(valueColor);
				TextMid(centerX, bottomLeftY+rectHeight-valueFontSize, (char*)valueString.c_str(), SansTypeface, valueFontSize-2);
			}
			if(valueVertAlign == 'C')
			{
				setfill(backgroundColor);
				Rect(centerX-(0.9*rectWidth/2), centerY-valueFontSize/2-1, 0.9*rectWidth, valueFontSize);
				setfill(valueColor);
				TextMid(centerX, centerY-valueFontSize/2, (char*)valueString.c_str(), SansTypeface, valueFontSize-2);
			}
			if(valueVertAlign == 'B')
			{	
				setfill(backgroundColor);
				Rect(centerX-(0.9*rectWidth/2), bottomLeftY+borderWidth-1, 0.9*rectWidth, valueFontSize);
				setfill(valueColor);
				TextMid(centerX, bottomLeftY+borderWidth, (char*)valueString.c_str(), SansTypeface, valueFontSize-2);
			}
			lastUpdateTime = currentTime;
		}


	/*
	if(currentTime>=nextTime){
		if (numLines == 2)
		{
			StrokeWidth(0);
			setfill(backgroundColor);
			Rect(centerX-(rectWidth+borderWidth)/2, centerY, rectWidth-2*borderWidth, fontSize);
			char valueText[10];

			sprintf(valueText, formatSpecifier, value);
			setfill(labelColor);
			TextMid(centerX, centerY+1, valueText, SansTypeface, fontSize-1);
		}

	}
	*/
}

/* Button setters */
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
}
void Button::setValueColor(float color[4])
{
	valueColor[0] = color[0];
	valueColor[1] = color[1];
	valueColor[2] = color[2];
	valueColor[3] = color[3];
}
void Button::setTextColor(float color[4])
{
	textColor[0] = color[0];
	textColor[1] = color[1];
	textColor[2] = color[2];
	textColor[3] = color[3];
}
void Button::setBorder(float color[4], int width)		// Set border color, border width
{
	borderWidth = width;
	borderColor[0] = color[0];
	borderColor[1] = color[1];
	borderColor[2] = color[2];
	borderColor[3] = color[3];
	rectHeight = readoutHeight-borderWidth;
	rectWidth = readoutWidth - borderWidth;
	bottomLeftX = centerX - (rectWidth+borderWidth) / 2;
	bottomLeftY = centerY - (rectHeight+borderWidth) / 2;
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
}