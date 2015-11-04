/********************************/
/*		Readout Class			*/
/********************************/
#include <iostream>
#include "VG/openvg.h"		//
#include "VG/vgu.h"			//
#include "fontinfo.h"		// OpenVG
#include "shapes.h"			//
#include <cmath>			// Math (float remainder operation)
#include "Readout.h"
#include <stdio.h>
#include <bcm2835.h>

using namespace std;		// ??

/* Readout Constructor */
Readout::Readout(int cx, int cy, int w, int h, int lines)
{
	lastUpdateTime = 0;
	centerX = cx;
	centerY = cy;
	readoutWidth = w;
	readoutHeight = h;
	numLines = 2;

	valueColor = new float[4]{0,1,0,1};
	labelColor = new float[4]{0,1,0,1};
	backgroundColor = new float[4]{0,0,0,1};
	borderColor = new float[4]{0,1,0,1};;
	// Default graphics options if setters are not called
	borderWidth = 4;
	label = "--";
	rectHeight = readoutHeight-borderWidth;
	rectWidth = readoutWidth - borderWidth;

	bottomLeftX = centerX - (rectWidth+borderWidth) / 2;
	bottomLeftY = centerY - (rectHeight+borderWidth) / 2;

	formatSpecifier = new char[10];
	int i=0;
	for(;i<10;i++)
	{
		formatSpecifier[i] = 0;
	}
	fontSize = 0.9 * (rectHeight - borderWidth) / 2;

	labelAlign = 'C';
	valueAlign = 'C';

}
/* Readout draw */
void Readout::draw(void)
{
	setfill(backgroundColor);
	StrokeWidth(borderWidth);
	setstroke(borderColor);
	Rect(bottomLeftX, bottomLeftY, rectWidth, rectHeight);
	setfill(labelColor);
	int labelFontSize = fontSize;		// Reduce label font size if need be
	int textWidth = TextWidth((char*)label.c_str(), SansTypeface, labelFontSize);
	while(textWidth > 0.9*rectWidth)
	{
		labelFontSize--;
		textWidth = TextWidth((char*)label.c_str(), SansTypeface, labelFontSize);
	}
	if(numLines == 2)
		TextMid(centerX, centerY - fontSize, (char*)label.c_str(), SansTypeface, labelFontSize);
}

void Readout::setDecPlaces(int dec)						// Set number of digits before & after decimal
{
	decPlaces = dec;
}

/* Readout update */
void Readout::update(float value)
{
	uint64_t currentTime = bcm2835_st_read();
	if(desiredRefreshRate == 0)	desiredRefreshRate = 5;
	uint64_t nextTime = lastUpdateTime + (1000000/desiredRefreshRate);
	if(currentTime>=nextTime){
		if (numLines == 2)
		{
			StrokeWidth(0);
			setfill(backgroundColor);
			Rect(centerX-(rectWidth+borderWidth)/2, centerY, rectWidth-2*borderWidth, fontSize);
			char valueText[10];

			// Shouldnt have to set format specifier here!! (but it doesnt work elsewhere...)
			char decPlacesText[10];
			sprintf(decPlacesText, "%d", decPlaces);
			string formatSpecifierString = "%.";
			formatSpecifierString.append(decPlacesText);
			formatSpecifierString.append("f");
			formatSpecifier = (char*)formatSpecifierString.c_str();
			sprintf(valueText, formatSpecifier, value);
			setfill(labelColor);
			TextMid(centerX, centerY+1, valueText, SansTypeface, fontSize-1);
		}
		lastUpdateTime = currentTime;
	}
	

}

/* Readout setters */
void Readout::setBackgroundColor(float color[4])
{
	backgroundColor[0] = color[0];
	backgroundColor[1] = color[1];
	backgroundColor[2] = color[2];
	backgroundColor[3] = color[3];
}
void Readout::setBorder(float color[4], int width)		// Set border color, border width
{
	borderWidth = width;
	borderColor[0] = color[0];
	borderColor[1] = color[1];
	borderColor[2] = color[2];
	borderColor[3] = color[3];
}

void Readout::setRefreshRate(int rate)		// Set desired refresh frequency (Hz)
{
	desiredRefreshRate = rate;
}
void Readout::alignValue(char align)					// 'L', 'R', 'C' for left, right, center value alignment
{
	valueAlign = align;
}
void Readout::alignLabel(char align)					// 'L', 'R', 'C' for left, right, center label alignment
{
	labelAlign = align;
}
void Readout::setLabel(string lbl)						// Set readout label
{
	label.assign(lbl);
}