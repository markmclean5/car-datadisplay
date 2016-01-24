/********************************/
/*		TextView Class			*/
/********************************/
#include <iostream>
#include "VG/openvg.h"		//
#include "VG/vgu.h"			//
#include "fontinfo.h"		// OpenVG
#include "shapes.h"			//
#include <cmath>			// Math (float remainder operation)
#include "TextView.h"
#include <stdio.h>
#include <bcm2835.h>
#include <locale.h>
#include <config4cpp/Configuration.h>
#include "parsingUtilities.h"

using namespace std;
using namespace config4cpp;

TextView::TextView(int cx, int cy, int w, int h, string identifier) {
textViewIdentifier = identifier;
centerX = cx;
centerY = cy;
width = w;
height = h;
setRectangular();
setRectWidthHeight(width, height);	// Called by derived class to set rectangular touch area size
setRectCenter(centerX, centerY);					// Called by derived class to set rectangular touch area bottom left corner
currentLine = 0;
cornerRadius = 0;
configure(identifier);
}

void TextView::configure(string ident) {
	setlocale(LC_ALL, "");
	Configuration * cfg = Configuration::create();
	try {
		cfg->parse("/home/pi/openvg/client/testConfig");
		string textViewName = ident;
		cornerRadius = parseInt(cfg, textViewName, "cornerRadius");
		borderWidth = parseInt(cfg, textViewName, "borderWidth");
		rectHeight = height-borderWidth;
		rectWidth = width - borderWidth;
		bottomLeftX = centerX - (rectWidth+borderWidth/2) / 2;
		bottomLeftY = centerY - (rectHeight+borderWidth/2) / 2;
		numLines = parseInt(cfg, textViewName, "numLines");
		if(numLines==0) numLines = 1;
		textViewData = new string[numLines];

		parseColor(cfg, textViewName, textColor, "textColor");
		textColorAlpha = textColor[3];
		lineColors = new float*[numLines];
		for(int i = 0; i<numLines; i++) {
			lineColors[i] = new float[4];
			lineColors[i] = textColor;
		}
		fontSize = rectHeight/(numLines+2);
		textPadding = (2*(rectHeight/numLines))/(numLines);
		parseColor(cfg, textViewName, borderColor, "borderColor");
		borderColorAlpha = borderColor[3];
		parseColor(cfg, textViewName, backgroundColor, "backgroundColor");
		backgroundColorAlpha = backgroundColor[3];
		
		setPressDebounce(parseInt(cfg, textViewName, "pressDebounce"));
	}catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
	cfg->destroy();

}

void TextView::update(void) {
	updateVisuals();
	// Handle movement: current position is not desired position
	if(centerX != getDesiredPosX() || centerY != getDesiredPosY()) {
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
	}
	setfill(backgroundColor);
	StrokeWidth(borderWidth);
	setstroke(borderColor);
	if(cornerRadius == 0) Rect(bottomLeftX, bottomLeftY, rectWidth, rectHeight);
	else {
		float cornerHeight = 0.01 * cornerRadius * rectWidth;
		Roundrect(bottomLeftX, bottomLeftY, rectWidth, rectHeight, cornerRadius, cornerRadius);
	}
	int idx = 0;
	for(;idx<currentLine;idx++) {
		setfill(lineColors[idx]);
		Text(bottomLeftX + cornerRadius, bottomLeftY+rectHeight-(idx+1)*fontSize - (idx+1)*textPadding, (char*)textViewData[idx].c_str(), SansTypeface, fontSize);
	}	
}

void TextView::addNewLine (string line) {
	if(currentLine <= numLines-1){
		textViewData[currentLine].assign(line);
		lineColors[currentLine] = textColor;
	}
	else {
		int idx = 0;
		for(;idx<numLines-1;idx++)
		{
			textViewData[idx].assign(textViewData[idx+1]);
			lineColors[idx] = lineColors[idx+1];
		}
		textViewData[currentLine-1].assign(line);
		lineColors[currentLine-1] = textColor;
	}
	if(currentLine <= numLines-1)
		currentLine++;
}

void TextView::clear(void)
{
	for(int line=0; line<numLines; line++) {
		textViewData[line].clear();
		currentLine = 0;
	}
}

void TextView::addNewLine (string line, float* color) {
	if(currentLine <= numLines-1){
		textViewData[currentLine].assign(line);
		lineColors[currentLine] = color;
	}
	else {
		int idx = 0;
		for(;idx<numLines-1;idx++)
		{
			textViewData[idx].assign(textViewData[idx+1]);
			lineColors[idx] = lineColors[idx+1];
		}
		textViewData[currentLine-1].assign(line);
		lineColors[currentLine-1] = color;
	}
	if(currentLine <= numLines-1)
		currentLine++;
}