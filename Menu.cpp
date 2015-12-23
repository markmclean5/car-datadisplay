/********************************/
/*			Menu Class			*/
/********************************/
#include <iostream>
#include "VG/openvg.h"		//
#include "VG/vgu.h"			//
#include "fontinfo.h"		// OpenVG
#include "shapes.h"			//
#include <cmath>			// Math (float remainder operation)
#include <stdio.h>
#include <bcm2835.h>
#include <locale.h>
#include <config4cpp/Configuration.h>
#include "parsingUtilities.h"

#include "Menu.h"
using namespace std;
using namespace config4cpp;

Menu::Menu(int cx, int cy, int w, int h, string identifier) {
	menuIdentifier = identifier;
	centerX = cx;
	centerY = cy;
	width = w;
	height = h;
	setRectangular();
	setRectWidthHeight(width, height);	// Called by derived class to set rectangular touch area size
	setRectCenter(centerX, centerY);	// Called by derived class to set rectangular touch area bottom left corner
	cornerRadius = 0;
	configure(menuIdentifier);
	isHorizontal = true;
}

void Menu::configure(string ident) {
	setlocale(LC_ALL, "");
	Configuration * cfg = Configuration::create();
	try {
		cfg->parse("testConfig");
		string menuName = ident;
		cornerRadius = parseInt(cfg, menuName, "cornerRadius");
		borderWidth = parseInt(cfg, menuName, "borderWidth");
		rectHeight = height - borderWidth;
		rectWidth = width - borderWidth;
		bottomLeftX = centerX - (rectWidth+borderWidth/2) / 2;
		bottomLeftY = centerY - (rectHeight+borderWidth/2) / 2;
		parseColor(cfg, menuName, borderColor, "borderColor");
		borderColorAlpha = borderColor[3];
		parseColor(cfg, menuName, backgroundColor, "backgroundColor");
		backgroundColorAlpha = backgroundColor[3];
		setPressDebounce(parseInt(cfg, menuName, "pressDebounce"));
		isHorizontal = parseBool(cfg, menuName, "isHorizontal");

		numButtons = parseInt(cfg, menuName, "numButtons");
		buttonPadding = parseInt(cfg, menuName, "buttonPadding");
		if(numButtons==0) numButtons = 1;
		if(isHorizontal) {
			buttonWidth = (rectWidth - buttonPadding*(numButtons+1))/numButtons; 
			buttonHeight = rectHeight - 2*buttonPadding;
		}

		int buttonCenterX = centerX - rectWidth/2 + buttonPadding + buttonWidth/2;
		int buttonCenterY = centerY;
		int offsetX = buttonPadding+ buttonWidth;
		int offsetY = 0;

		string buttonScope = "button";
		int currentButton = 1;
		for(;currentButton<=numButtons;currentButton++) {

			menuButtons.push_back(Button(buttonCenterX, buttonCenterY, buttonWidth, buttonHeight, menuName+"."+ buttonScope + std::to_string(currentButton)));
			cout << "Config string:"<<menuName+"."+ buttonScope + std::to_string(currentButton) << endl;
			buttonCenterX+=offsetX;
			buttonCenterY+=offsetY;
		}

		for(int idx = 0; idx < menuButtons.size(); idx++) {
			menuButtons[idx].touchEnable();
		}

	}catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
	cfg->destroy();

}

void Menu::update(touch_t menuTouch) {
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
	}
	setfill(backgroundColor);
	StrokeWidth(borderWidth);
	setstroke(borderColor);
	if(cornerRadius == 0) Rect(bottomLeftX, bottomLeftY, rectWidth, rectHeight);
	else {
		float cornerHeight = 0.01 * cornerRadius * rectWidth;
		Roundrect(bottomLeftX, bottomLeftY, rectWidth, rectHeight, cornerRadius, cornerRadius);
	}
	for(int idx = 0;idx<menuButtons.size();idx++) {
		menuButtons[idx].update();
		menuButtons[idx].updateTouch(menuTouch);
	}
}
