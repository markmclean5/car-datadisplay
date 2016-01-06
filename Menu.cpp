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
#include <memory>

#include "Menu.h"
using namespace std;
using namespace config4cpp;

/*	Menu constructor: creates menu about provided center coordinate with provided size
	and configures menu using identifier string. */
Menu::Menu(int cx, int cy, int w, int h, string identifier) {
	menuIdentifier = identifier;
	centerX = cx;
	centerY = cy;
	width = w;
	height = h;
	// Configure menu as TouchableObject
	setRectangular();
	setRectWidthHeight(width, height);
	setRectCenter(centerX, centerY);
	// Default property values
	cornerRadius = 0;
	isHorizontal = true;
	hideable = false;
	hidden = false;
	menuSelectMode = "manual";
	configureButtons = false;


	timedSelectionStart = bcm2835_st_read();
	timedSelectionEnd = 0;

	// Call configure
	configure(menuIdentifier);
}


/* Menu configuration: capture menu properties from configuration file entry with the provided string */ 
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
		pressDebounce = parseInt(cfg, menuName, "pressDebounce");
		setPressDebounce(pressDebounce);
		isHorizontal = parseBool(cfg, menuName, "isHorizontal");
		numButtons = parseInt(cfg, menuName, "numButtons");
		buttonNames = new string[numButtons];
		buttonCfgText = new string[numButtons];
		buttonSelectStates = new bool[numButtons];
		menuButtons.reserve(numButtons);
		buttonPadding = parseInt(cfg, menuName, "buttonPadding");
		menuSelectMode = parseString(cfg, menuName, "selectMode");
		if(menuSelectMode.compare("timed") == 0)
			timedSelectDuration = parseInt(cfg, menuName, "timedSelectDuration");
		
		configureButtons = parseBool(cfg, menuName, "configureButtons");
		if(configureButtons) {
			parseColor(cfg, menuName, buttonBackgroundColor, "buttonBackgroundColor");
			parseColor(cfg, menuName, buttonSelectedBackgroundColor, "buttonSelectedBackgroundColor");
			parseColor(cfg, menuName, buttonBorderColor, "buttonBorderColor");
			parseColor(cfg, menuName, buttonSelectedBorderColor, "buttonSelectedBorderColor");
			parseColor(cfg, menuName, buttonTextColor, "buttonTextColor");
			parseColor(cfg, menuName, buttonSelectedTextColor, "buttonSelectedTextColor");
			buttonBorderWidth = parseInt(cfg, menuName, "buttonBorderWidth");
			buttonSelectedBorderWidth = parseInt(cfg, menuName, "buttonSelectedBorderWidth");
			buttonCornerRadius = parseInt(cfg, menuName, "buttonCornerRadius");
			for(int i=1; i<=numButtons; i++) {
				buttonNames[i-1] = parseString(cfg, menuName, "buttonName"+std::to_string(i));
				buttonCfgText[i-1] = parseString(cfg, menuName, "buttonText"+std::to_string(i));
			}
		}
		hideable = parseBool(cfg, menuName, "hideable");
		if(hideable) {
			hideDeltaX = parseInt(cfg, menuName, "hideDeltaX");
			hideDeltaY = parseInt(cfg, menuName, "hideDeltaY");
			hideFade = parseInt(cfg, menuName, "hideFade");
			hideDuration = parseInt(cfg, menuName, "hideDuration");
		}
		if(numButtons==0) numButtons = 1;
		int buttonCenterX, buttonCenterY;
		int offsetX, offsetY;
		if(isHorizontal) {
			buttonWidth = (rectWidth - buttonPadding*(numButtons+1))/numButtons; 
			buttonHeight = rectHeight - 2*buttonPadding;
			buttonCenterX = centerX - rectWidth/2 + buttonPadding + buttonWidth/2;
			buttonCenterY = centerY;
			offsetX = buttonPadding+ buttonWidth;
			offsetY = 0;
		}
		else {
			buttonWidth = rectWidth - 2*buttonPadding;
			buttonHeight = (rectHeight - buttonPadding*(numButtons+1))/numButtons;
			buttonCenterX = centerX;
			buttonCenterY = centerY + rectHeight/2 -buttonPadding - buttonHeight/2;
			offsetX = 0;
			offsetY = - buttonPadding - buttonHeight;
		}
		string buttonScope = "button";
		int currentButton = 1;
		for(;currentButton<=numButtons;currentButton++) {
			if(configureButtons){
				menuButtons.emplace_back(buttonCenterX, buttonCenterY, buttonWidth, buttonHeight);
				int b = menuButtons.size()-1;
				menuButtons[b].setSelectable();
				menuButtons[b].setBorder(buttonBorderColor, buttonBorderWidth);
				menuButtons[b].setSelectedBorder(buttonSelectedBorderColor, buttonSelectedBorderWidth);
				menuButtons[b].setCornerRadius(buttonCornerRadius);
				menuButtons[b].setBackgroundColor(buttonBackgroundColor);
				menuButtons[b].setSelectedBackgroundColor(buttonSelectedBackgroundColor);
				menuButtons[b].enableText('C');
				menuButtons[b].setTextColor(buttonTextColor);
				menuButtons[b].setSelectedTextColor(buttonSelectedTextColor);
				menuButtons[b].setName(buttonNames[currentButton-1]);
				menuButtons[b].setText(buttonCfgText[currentButton-1]);
				menuButtons[b].setPressDebounce(pressDebounce);
			}
			else{
				menuButtons.emplace_back(buttonCenterX, buttonCenterY, buttonWidth, buttonHeight, menuName+"."+ buttonScope + std::to_string(currentButton));
			}
			buttonCenterX+=offsetX;
			buttonCenterY+=offsetY;
		}
		
		// Loop through all menu button elements, also set populate Button ID array to minimize future vector searching
		for(int idx = 0; idx < menuButtons.size(); idx++) {
			menuButtons[idx].touchEnable();
			buttonNames[idx] = menuButtons[idx].getName();
		}
	}catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
	cfg->destroy();
}

/* Menu update function: updates buttons, states, and draws menu */
void Menu::update(touch_t menuTouch) {
	uint64_t currentTime = bcm2835_st_read();
	updateVisuals();
	updateTouch(menuTouch);
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
		if((menuSelectMode.compare("timed") == 0) && (currentTime >= timedSelectionEnd) && menuButtons[idx].isSelected())
			menuButtons[idx].deselect();
		buttonSelectStates[idx] = menuButtons[idx].isSelected();
		menuButtons[idx].update();
		menuButtons[idx].updateTouch(menuTouch);
	}
}

bool Menu::isButtonPressed(string name) {
	return menuButtons[getVectorIndex(name)].isPressed();
}

/* Returns the selected state of the first button which matches the provided name */
bool Menu::isButtonSelected(string name) {
	return menuButtons[getVectorIndex(name)].isSelected();
}

/* Selects the first button which matches the provided name */
void Menu::selectButton(string name) {
	if(menuSelectMode.compare("radio") == 0) {
		for(int idx = 0; idx<menuButtons.size(); idx++) {
			if(buttonSelectStates[idx]) {
				menuButtons[idx].deselect();
				cout << "Deselecting: " << idx << endl; 
			}
		}
		menuButtons[getVectorIndex(name)].select();
		
	}
	else if(menuSelectMode.compare("timed") == 0) {
		uint64_t currentTime = bcm2835_st_read();
		if(currentTime >= timedSelectionEnd) {
			timedSelectionStart = currentTime;
			timedSelectionEnd = timedSelectionStart + (1000*timedSelectDuration);
			menuButtons[getVectorIndex(name)].select();
		}
		
	}

}

/* Deselects the first button which matches the provided name */
void Menu::deselectButton(string name) {
	menuButtons[getVectorIndex(name)].deselect();
}

/* Returns the index of the first item in the menu buttons vector which matches the provided name */
int Menu::getVectorIndex(string name) {
	int idx = 0;
	for(;idx<menuButtons.size();idx++) {
		if(name.compare(buttonNames[idx]) == 0) break;
	}
	//cout << "String: " << name << " Index: " << idx << endl;
	return idx;
}


/* Called to hide the menu if not hidden (move fade to configured coordinate and alpha) */
void Menu::hide(void) {
	if(!hidden && hideable && !isMoving()) {
		move(hideDeltaX, hideDeltaY, hideDuration, "AAA");
		if(hideFade != 0)
			fade(hideFade, hideDuration, "AAA");
		for(int idx = 0; idx<menuButtons.size(); idx++) {
			menuButtons[idx].move(hideDeltaX, hideDeltaY, hideDuration, "AAA");
			if(hideFade != 0)
				menuButtons[idx].fade(hideFade, hideDuration, "AAA");
		}
	hidden = true;
	}

}
/* Called to unhide the menu if hidden (move back and fade to original alpha) */
void Menu::unhide(void) { 
	if(hidden && hideable && !isMoving()) {
		move(-hideDeltaX, -hideDeltaY, hideDuration, "AAA");
		if(hideFade != 0)
			fade(0, hideDuration, "AAA");

		for(int idx = 0; idx<menuButtons.size(); idx++) {
			menuButtons[idx].move(-hideDeltaX, -hideDeltaY, hideDuration, "AAA");
			if(hideFade != 0)
				menuButtons[idx].fade(0, hideDuration, "AAA");
		}
	}
	hidden = false;
}

/* Gets menu current hide state */
bool Menu::isHidden(void) {
	return hidden;
}
/* Loop through buttons in menu, returns name of first button which is pressed */
string Menu::getPressedButtonName(void) {
	string name = "";
	for(int idx = 0; idx<menuButtons.size(); idx++)
	{
		if(menuButtons[idx].isPressed()) {
			name.append(menuButtons[idx].getName());
			break;
		}
	}
	return name;
}