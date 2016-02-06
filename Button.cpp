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
#include "parsingUtilities.h"
#include <locale.h>
#include <config4cpp/Configuration.h>


using namespace std;
using namespace config4cpp;

string valueString = "";

/* Button Constructor: load size and location from config file*/
Button::Button(string identifier) {
	buttonIdentifier = identifier;
	givenSizeAndLocation = false;
	lastUpdateTime = 0;
	valueColor = new float[4]{0,1,0,1};
	valueColorAlpha = valueColor[3];
	textColor = new float[4]{0,1,0,1};
	textColorAlpha = textColor[3];
	containsText = false;
	containsValue = false;
	cornerRadius = 0;
	configure(buttonIdentifier);
	setRectangular();
	setRectWidthHeight(readoutWidth, readoutHeight);	// Called by derived class to set rectangular touch area size
	setRectCenter(centerX, centerY);					// Called by derived class to set rectangular touch area bottom left corner
	bufferSaved = false;
}

/* Button Constructor: Use given location and size, load properties from config file*/
Button::Button(int cX, int cY, int w, int h, string identifier) {
	buttonIdentifier = identifier;
	givenSizeAndLocation = true;
	centerX = cX;
	centerY = cY;
	readoutWidth = w;
	readoutHeight = h;
	lastUpdateTime = 0;
	valueColor = new float[4]{0,1,0,1};
	valueColorAlpha = valueColor[3];
	textColor = new float[4]{0,1,0,1};
	textColorAlpha = textColor[3];
	containsText = false;
	containsValue = false;
	cornerRadius = 0;
	selectable = false;
	selected = false;
	configure(buttonIdentifier);
	setRectangular();
	setRectWidthHeight(readoutWidth, readoutHeight);	// Called by derived class to set rectangular touch area size
	setRectCenter(centerX, centerY);					// Called by derived class to set rectangular touch area bottom left corner
}

/* Button Constructor: Use given location and size, call setters for everything else*/
Button::Button(int cX, int cY, int w, int h) {
	buttonIdentifier="";
	givenSizeAndLocation = true;
	centerX = cX;
	centerY = cY;
	readoutWidth = w;
	readoutHeight = h;
	lastUpdateTime = 0;
	valueColor = new float[4]{0,1,0,1};
	valueColorAlpha = valueColor[3];
	textColor = new float[4]{0,1,0,1};
	textColorAlpha = textColor[3];
	containsText = false;
	containsValue = false;
	cornerRadius = 0;
	selectable = false;
	selected = false;
	setRectangular();
	setRectWidthHeight(readoutWidth, readoutHeight);	// Called by derived class to set rectangular touch area size
	setRectCenter(centerX, centerY);					// Called by derived class to set rectangular touch area bottom left corner
	
	//bufferImage = vgCreateImage(VG_sABGR_8888, 800, 480, VG_IMAGE_QUALITY_BETTER);
}

/* Button configure method */
void Button::configure(string ident) {
	cout << "Configuring button: " << ident << endl;
	setlocale(LC_ALL, "");
	Configuration * cfg = Configuration::create();
	try {
		cfg->parse("/home/pi/openvg/client/testConfig");
		string buttonName = ident;
		if(!givenSizeAndLocation){
			readoutWidth = parseInt(cfg, buttonName, "width");
			readoutHeight = parseInt(cfg, buttonName, "height");
			centerX = parseInt(cfg, buttonName, "centerX");
			centerY = parseInt(cfg, buttonName, "centerY");
		}
		name = parseString(cfg, buttonName, "name");
		cornerRadius = parseInt(cfg, buttonName, "cornerRadius");
		borderWidth = parseInt(cfg, buttonName, "borderWidth");
		rectHeight = readoutHeight-borderWidth;
		rectWidth = readoutWidth - borderWidth;
		bottomLeftX = centerX - (rectWidth+borderWidth/2) / 2;
		bottomLeftY = centerY - (rectHeight+borderWidth/2) / 2;
		parseColor(cfg, buttonName, borderColor, "borderColor");
		parseColor(cfg, buttonName, backgroundColor, "backgroundColor");
		borderColorAlpha = borderColor[3];
		backgroundColorAlpha = backgroundColor[3];
		containsText = parseBool(cfg, buttonName, "enableText");
		if(containsText) {
			string textAlign = parseString(cfg, buttonName, "textAlign");
			textVertAlign = textAlign.at(0);
			parseColor(cfg, buttonName, textColor, "textColor");
			text = parseString(cfg, buttonName, "defaultText");			
		}
		containsValue = parseBool(cfg, buttonName, "enableValue");
		if(containsValue) {
			string valueAlign = parseString(cfg, buttonName, "valueAlign");
			valueVertAlign = valueAlign.at(0);
			parseColor(cfg, buttonName, valueColor, "valueColor");
			desiredRefreshRate = parseInt(cfg, buttonName, "valueRefreshRate");
			setValueDecPlaces(parseInt(cfg, buttonName, "valueDecPlaces"));
		}

		selectable = parseBool(cfg, buttonName, "selectable");
		if(selectable) {
			parseColor(cfg, buttonName, selectedBackgroundColor, "selectedBackgroundColor");
			selectedBackgroundColorAlpha = selectedBackgroundColor[3];
			parseColor(cfg, buttonName, selectedBorderColor, "selectedBorderColor");
			selectedBorderColorAlpha = selectedBorderColor[3];
			selectedBorderWidth = parseInt(cfg, buttonName, "selectedBorderWidth");
			if(containsText) {
				parseColor(cfg, buttonName, selectedTextColor, "selectedTextColor");
				selectedTextColorAlpha = selectedTextColor[3];
			}
			if(containsValue) {
				parseColor(cfg, buttonName, selectedValueColor, "selectedValueColor");
				selectedValueColorAlpha = selectedValueColor[3];
			}
		}
		setPressDebounce(parseInt(cfg, buttonName, "pressDebounce"));
	}catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
	cfg->destroy();
}

/* Button update */
void Button::update(void) {
	//cout << "Button update called for : " << buttonIdentifier << endl;
	updateVisuals();
	// Handle movement: current position is not desired position
	if(centerX != getDesiredPosX() || centerY != getDesiredPosY()) {
		bufferSaved = false;
		centerX = getDesiredPosX();
		centerY = getDesiredPosY();
		bottomLeftX = centerX - (rectWidth+borderWidth/2) / 2;
		bottomLeftY = centerY - (rectHeight+borderWidth/2) / 2;
	}


	if(selected) {
		setfill(selectedBackgroundColor);
		StrokeWidth(selectedBorderWidth);
		setstroke(selectedBorderColor);
	}
	else {
		setfill(backgroundColor);
		StrokeWidth(borderWidth);
		setstroke(borderColor);
	}
	if(cornerRadius == 0) Rect(bottomLeftX, bottomLeftY, rectWidth, rectHeight);
	else {
		float cornerHeight = 0.01 * cornerRadius * rectWidth;
		Roundrect(bottomLeftX, bottomLeftY, rectWidth, rectHeight, cornerRadius, cornerRadius);
	}
	if(containsText) {
		if(selected) setfill(selectedTextColor);
		else setfill(textColor);
		StrokeWidth(0);
		textFontSize = (rectHeight-borderWidth)/2;
		int textWidth = TextWidth((char*)text.c_str(), SansTypeface, textFontSize);
		while(textWidth > 0.9*rectWidth) {
			textFontSize-=0.25;
			textWidth = TextWidth((char*)text.c_str(), SansTypeface, textFontSize);
		}
		if(textVertAlign == 'T')
		TextMid(centerX, bottomLeftY+rectHeight-textFontSize, (char*)text.c_str(), SansTypeface, textFontSize);
		if(textVertAlign == 'C')
			TextMid(centerX, centerY-textFontSize/2, (char*)text.c_str(), SansTypeface, textFontSize);
		if(textVertAlign == 'B')
			TextMid(centerX, bottomLeftY+borderWidth, (char*)text.c_str(), SansTypeface, textFontSize);
	}
	if(containsValue) {
		if(selected) setfill(selectedValueColor);
		else setfill(valueColor);
		if(valueVertAlign == 'T')
			TextMid(centerX, bottomLeftY+rectHeight-valueFontSize, (char*)valueString.c_str(), SansTypeface, valueFontSize-2);
		if(valueVertAlign == 'C')
			TextMid(centerX, centerY-valueFontSize/2, (char*)valueString.c_str(), SansTypeface, valueFontSize-2);
		if(valueVertAlign == 'B')
			TextMid(centerX, bottomLeftY+borderWidth, (char*)valueString.c_str(), SansTypeface, valueFontSize-2);
	}
	



}

void Button::setCornerRadius(int rad)
{
	cornerRadius = rad;
	bufferSaved = false;
}

void Button::setValueDecPlaces(int dec)						// Set number of digits before & after decimal
{
	valueDecPlaces = dec;
	char decPlacesText[10];
	sprintf(decPlacesText, "%d", valueDecPlaces);
	formatSpecifierString = "%.";
	formatSpecifierString.append(decPlacesText);
	formatSpecifierString.append("f");
	bufferSaved = false;
}
void Button::setBackgroundColor(float color[4])
{
	backgroundColor[0] = color[0];
	backgroundColor[1] = color[1];
	backgroundColor[2] = color[2];
	backgroundColor[3] = color[3];
	backgroundColorAlpha = backgroundColor[3];
	bufferSaved = false;
}
void Button::setValueColor(float color[4])
{
	valueColor[0] = color[0];
	valueColor[1] = color[1];
	valueColor[2] = color[2];
	valueColor[3] = color[3];
	valueColorAlpha = valueColor[3];
	bufferSaved = false;
}
void Button::setTextColor(float color[4])
{
	textColor[0] = color[0];
	textColor[1] = color[1];
	textColor[2] = color[2];
	textColor[3] = color[3];
	textColorAlpha = textColor[3];
	bufferSaved = false;
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
	bufferSaved = false;
}
void Button::setValueRefreshRate(int rate) {			// Set desired refresh frequency (Hz)
	desiredRefreshRate = rate;
}
void Button::enableValue(char v) {
	valueVertAlign = v;
	containsValue = true;
	bufferSaved = false;
}

void Button::enableText(char v) {
	textVertAlign = v;
	containsText = true;
	bufferSaved = false;
}

/* Set button text */
void Button::setText(string txt) {						// Set readout label
	text.assign(txt);
	bufferSaved = false;
}

/* Set button value */
void Button::setValue(float val) {						// Set readout label
	value = val;
	uint64_t currentTime = bcm2835_st_read();
	if(desiredRefreshRate == 0)	desiredRefreshRate = 5;
	uint64_t nextTime = lastUpdateTime + (1000000/desiredRefreshRate);
	if(containsValue && currentTime>=nextTime) {
		setfill(valueColor);
		StrokeWidth(0);
		valueFontSize = (rectHeight-borderWidth)/2;
		char valueText[10];
		sprintf(valueText, (char*)formatSpecifierString.c_str(), value);
		valueString=valueText;
		lastUpdateTime = currentTime;
	}
	int valueWidth = TextWidth((char*)valueString.c_str(), SansTypeface, valueFontSize);
	while(valueWidth > 0.9*rectWidth) {
		valueFontSize--;
		valueWidth = TextWidth((char*)valueString.c_str(), SansTypeface, valueFontSize);
	}
	bufferSaved = false;
}

string Button::getIdentifier(void) {
	return buttonIdentifier;
}

void Button::setName(string name) {
	buttonName = name;
}

string Button::getName(void) {
	return buttonName;
}

void Button::select(void) {
	if(selectable && (selected == false)) {
		selected = true;
		rectHeight = readoutHeight-selectedBorderWidth;
		rectWidth = readoutWidth - selectedBorderWidth;
		bottomLeftX = centerX - (rectWidth+selectedBorderWidth/2) / 2;
		bottomLeftY = centerY - (rectHeight+selectedBorderWidth/2) / 2;
		bufferSaved = false;
	}
}

void Button::deselect(void) {
	if(selected) {
		selected = false;
		rectHeight = readoutHeight-borderWidth;
		rectWidth = readoutWidth - borderWidth;
		bottomLeftX = centerX - (rectWidth+borderWidth/2) / 2;
		bottomLeftY = centerY - (rectHeight+borderWidth/2) / 2;
		bufferSaved = false;
	}
	
}

bool Button::isSelected(void) {
	return selected;
}

void Button::setSelectedBorder(float color[4], int width) {
	selectedBorderColor[0] = color[0];
	selectedBorderColor[1] = color[1];
	selectedBorderColor[2] = color[2];
	selectedBorderColor[3] = color[3];
	selectedBorderColorAlpha = selectedBorderColor[3];
	selectedBorderWidth = width;
	bufferSaved = false;

}

void Button::setSelectedBackgroundColor(float color[4]) {
	selectedBackgroundColor[0] = color[0];
	selectedBackgroundColor[1] = color[1];
	selectedBackgroundColor[2] = color[2];
	selectedBackgroundColor[3] = color[3];
	selectedBackgroundColorAlpha = selectedBackgroundColor[3];
	bufferSaved = false;
}


void Button::setSelectedTextColor(float color[4]) {
	selectedTextColor[0] = color[0];
	selectedTextColor[1] = color[1];
	selectedTextColor[2] = color[2];
	selectedTextColor[3] = color[3];
	selectedTextColorAlpha = selectedTextColor[3];
	bufferSaved = false;
}


void Button::setSelectable(void) {
	selectable = true;
}
