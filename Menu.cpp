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
	bufferImage = vgCreateImage(VG_sABGR_8888, 800, 480, VG_IMAGE_QUALITY_BETTER);
	bufferSaved = false;
	titled = false;
	scrollable = false;
	title = "";
	topMenuItemIndex = 0;
	configure(menuIdentifier);		// Configure the menu
}

/* Menu update function: updates buttons, states, and draws menu */
void Menu::update(touch_t menuTouch) {
	uint64_t currentTime = bcm2835_st_read();
	updateVisuals();
	updateTouch(menuTouch);
	bool useBuffer = true;
	// Handle movement: current position is not desired position
	if(centerX != getDesiredPosX() || centerY != getDesiredPosY()) {
		centerX = getDesiredPosX();
		centerY = getDesiredPosY();
		bottomLeftX = centerX - (rectWidth+borderWidth/2) / 2;
		bottomLeftY = centerY - (rectHeight+borderWidth/2) / 2;
		bufferSaved = false;
	}
	// Handle fade:
	if(getDesiredFadePercentage() != 0) {
		float alphaScalar = (100. - getDesiredFadePercentage()) / 100.;
		backgroundColor[3] = backgroundColorAlpha * alphaScalar;
		borderColor[3] = borderColorAlpha * alphaScalar;
		bufferSaved = false;
	}
	// Selection processing
	for(int idx = 0;idx<menuButtons.size();idx++) {
		if((menuSelectMode.compare("timed") == 0) && (currentTime >= timedSelectionEnd) && menuButtons[idx].isSelected()) {
			menuButtons[idx].deselect();
			bufferSaved = false;
		}
		buttonSelectStates[idx] = menuButtons[idx].isSelected();		
	}
	

	// Previous button pressed
	if(scrollable && prevButton && previousBtn->isPressed()) {
		cout << "prev btn pressed" << endl;
		uint64_t currentTime = bcm2835_st_read();
		timedSelectionStart = currentTime;
		timedSelectionEnd = timedSelectionStart + (1000*timedSelectDuration);
		previousBtn->select();

		if(menuItemsRemaining < totalItems){
			topMenuItemIndex -= scrollItems;
			menuItemsRemaining += scrollItems;
		}
		if(activeButtons < numButtons) activeButtons += scrollItems;
		int currentButton = 1;
		for(;currentButton<=activeButtons;currentButton++) {
			menuButtons[currentButton-1].setName(buttonNames[topMenuItemIndex+currentButton-1]);
			menuButtons[currentButton-1].setText(buttonCfgText[topMenuItemIndex+currentButton-1]);
		}
		bufferSaved = false;
	}

	// Next button pressed
	if(scrollable && nextButton && nextBtn->isPressed()) {
		uint64_t currentTime = bcm2835_st_read();
		timedSelectionStart = currentTime;
		timedSelectionEnd = timedSelectionStart + (1000*timedSelectDuration);
		nextBtn->select();
		if(menuItemsRemaining >= scrollItems){
			topMenuItemIndex += scrollItems;
			menuItemsRemaining -= scrollItems;
		}
		if(menuItemsRemaining < activeButtons) activeButtons = menuItemsRemaining;
		int currentButton = 1;
		for(;currentButton<=activeButtons;currentButton++) {
			menuButtons[currentButton-1].setName(buttonNames[topMenuItemIndex+currentButton-1]);
			menuButtons[currentButton-1].setText(buttonCfgText[topMenuItemIndex+currentButton-1]);
		}
		bufferSaved = false;
	}


	if(scrollable && prevButton && (menuSelectMode.compare("timed") == 0) && (currentTime >= timedSelectionEnd) && previousBtn->isSelected()) {
			previousBtn->deselect();
			bufferSaved = false;
	}
	if(scrollable && nextButton && (menuSelectMode.compare("timed") == 0) && (currentTime >= timedSelectionEnd) && nextBtn->isSelected()) {
			nextBtn->deselect();
			bufferSaved = false;
	}


	// Draw menu if buffer image is not saved or is out of date
	if(!bufferSaved) {

		cout << "buffer not saved, creating buffer" << endl;
		// Background & border
		setfill(backgroundColor);
		StrokeWidth(borderWidth);
		setstroke(borderColor);
		if(cornerRadius == 0) Rect(bottomLeftX, bottomLeftY, rectWidth, rectHeight);
		else {
			float cornerHeight = 0.01 * cornerRadius * rectWidth;
			Roundrect(bottomLeftX, bottomLeftY, rectWidth, rectHeight, cornerRadius, cornerRadius);
		}
		// Title
		if(titled) {
			int titleHeight = titlePercentHeight/100. * rectHeight;
			int titleBottonLeftX = centerX - rectWidth/2 + buttonPadding;
			int titleBottomLeftY = centerY + rectHeight/2 - buttonPadding - titleHeight;
			setfill(titleColor);
			if(titleFontSize > titleHeight) titleFontSize = titleHeight;
			TextMid(centerX, titleBottomLeftY + (titleHeight - titleFontSize)/2, (char*)title.c_str(), SansTypeface, titleFontSize);
		}
		// Buttons
		for(int idx = 0;idx<activeButtons;idx++) {
			menuButtons[idx].update();
		}
		if(scrollable && prevButton) previousBtn->update();
		if(scrollable && nextButton) nextBtn->update();
		// Save buffer image
		vgGetPixels(bufferImage, centerX-width/2, centerY-height/2, centerX - width/2, centerY - height/2, width, height);
		bufferSaved = true;
	}
	else vgDrawImage(bufferImage);


	for(int idx = 0;idx<activeButtons;idx++) {
		menuButtons[idx].updateTouch(menuTouch);
	}
	if(scrollable && prevButton) previousBtn->updateTouch(menuTouch);
	if(scrollable && nextButton) nextBtn->updateTouch(menuTouch);

}


/* Menu configuration: capture menu properties from configuration file entry with the provided string */ 
void Menu::configure(string ident) {
	setlocale(LC_ALL, "");
	Configuration * cfg = Configuration::create();
	cout << "Configuring menu: " << ident << endl;
	try {
		cfg->parse("/home/pi/openvg/client/testConfig");
		string menuName = ident;
		cornerRadius = parseInt(cfg, menuName, "cornerRadius");
		borderWidth = parseInt(cfg, menuName, "borderWidth");
		// Given position and border, determine size and start coordinate of menu rectangle
		rectHeight = height - borderWidth;
		rectWidth = width - borderWidth;
		bottomLeftX = centerX - (rectWidth+borderWidth/2) / 2;
		bottomLeftY = centerY - (rectHeight+borderWidth/2) / 2;
		// Colors, save initial alphas for fading later on
		parseColor(cfg, menuName, borderColor, "borderColor");
		borderColorAlpha = borderColor[3];
		parseColor(cfg, menuName, backgroundColor, "backgroundColor");
		backgroundColorAlpha = backgroundColor[3];
		// Title configuration
		titled = parseBool(cfg, menuName, "titled");
		if(titled) {
			parseColor(cfg, menuName, titleColor, "titleColor");
			titleColorAlpha = titleColor[3];
			title = parseString(cfg, menuName, "titleText");
			titlePercentHeight = parseInt(cfg, menuName, "titlePercentHeight");
			titleFontSize = parseInt(cfg, menuName, "titleFontSize");	
		}
		// Scrollable configuration
		scrollable = parseBool(cfg, menuName, "scrollable");
		if(scrollable) {
			cout << "scrollable menu!" << endl;
			totalItems = parseInt(cfg, menuName, "totalItems");
			scrollItems = parseInt(cfg, menuName, "scrollItems");
			scrollButtonPercentHeight = parseInt(cfg, menuName, "scrollButtonPercentHeight");
			prevButton = parseBool(cfg, menuName, "prevButton");
			prevButtonText = parseString(cfg, menuName, "prevButtonText");
			nextButton = parseBool(cfg, menuName, "nextButton");
			nextButtonText = parseString(cfg, menuName, "nextButtonText");
			topMenuItemIndex = 0;
			menuItemsRemaining = totalItems;
		}
		pressDebounce = parseInt(cfg, menuName, "pressDebounce");
		setPressDebounce(pressDebounce);
		isHorizontal = parseBool(cfg, menuName, "isHorizontal");
		numButtons = parseInt(cfg, menuName, "numButtons");
		// Set up arrays to store names, text, and select states for entire 
		if(!scrollable) totalItems = numButtons;
		buttonNames = new string[totalItems];
		buttonCfgText = new string[totalItems];
		buttonSelectStates = new bool[totalItems];
		activeButtons = numButtons;
		menuButtons.reserve(numButtons);
		buttonPadding = parseInt(cfg, menuName, "buttonPadding");
		// Menu select mode
		menuSelectMode = parseString(cfg, menuName, "selectMode");
		if(menuSelectMode.compare("timed") == 0)
			timedSelectDuration = parseInt(cfg, menuName, "timedSelectDuration");
		// Configure buttons - all menu buttons inherit the button_ proprties
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
			// Store names and text for all menu items
			for(int i=1; i<=totalItems; i++) {
				buttonNames[i-1] = parseString(cfg, menuName, "buttonName"+std::to_string(i));
				buttonCfgText[i-1] = parseString(cfg, menuName, "buttonText"+std::to_string(i));
			}
		}

		// Hide (move to "hidden" location, and fade)
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
		// Horizontal menu
		if(isHorizontal) {
			buttonWidth = (rectWidth - buttonPadding*(numButtons+1))/numButtons; 
			buttonHeight = rectHeight - 2*buttonPadding;
			buttonCenterX = centerX - rectWidth/2 + buttonPadding + buttonWidth/2;
			buttonCenterY = centerY;
			offsetX = buttonPadding+ buttonWidth;
			offsetY = 0;
		}
		// Vertical menu: supports title
		else {
			if(titled) {
				if(!scrollable)
					buttonHeight = (rectHeight - (rectHeight*(titlePercentHeight/100.)) - buttonPadding*(numButtons+1))/numButtons;
				else
					buttonHeight = (rectHeight - (rectHeight*(titlePercentHeight/100.) + (rectHeight*(scrollButtonPercentHeight/100.))) - buttonPadding*(numButtons+1))/numButtons;
				buttonCenterY = centerY + rectHeight/2 - 2*buttonPadding - buttonHeight/2 - rectHeight*(titlePercentHeight/100.);
			}
			else {
				buttonHeight = (rectHeight - buttonPadding*(numButtons+1))/numButtons;
				buttonCenterY = centerY + rectHeight/2 -buttonPadding - buttonHeight/2;
			}
			offsetX = 0;
			offsetY = - buttonPadding - buttonHeight;
			buttonCenterX = centerX;
			buttonWidth = rectWidth - 2*buttonPadding;
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
				menuButtons[b].touchEnable();
			}
			else
				menuButtons.emplace_back(buttonCenterX, buttonCenterY, buttonWidth, buttonHeight, menuName+"."+ buttonScope + std::to_string(currentButton));
			buttonCenterX+=offsetX;
			buttonCenterY+=offsetY;
		}

		if(scrollable) {
			cout << "creating scroll btns" << endl;
			int scrollButtonHeight = rectHeight*(scrollButtonPercentHeight/100.) - buttonPadding;
			int scrollButtonWidth = rectWidth/2 - 3*buttonPadding;
			int scrollButtonCenterY = centerY - rectHeight/2 + scrollButtonHeight/2 + buttonPadding;
			int prevButtonCenterX = centerX - rectWidth/2 + scrollButtonWidth/2 + buttonPadding;
			int nextButtonCenterX = centerX + rectWidth/2 - scrollButtonWidth/2 - buttonPadding;
			if(prevButton) {
				cout << "prev button" << endl;
				previousBtn = new Button(prevButtonCenterX, scrollButtonCenterY, scrollButtonWidth, scrollButtonHeight);
				cout << "created previous btn" << endl;
				previousBtn->setSelectable();
				cout << "configuring previous btn" << endl;
				previousBtn->setBorder(buttonBorderColor, buttonBorderWidth);
				previousBtn->setSelectedBorder(buttonSelectedBorderColor, buttonSelectedBorderWidth);
				previousBtn->setCornerRadius(buttonCornerRadius);
				previousBtn->setBackgroundColor(buttonBackgroundColor);
				previousBtn->setSelectedBackgroundColor(buttonSelectedBackgroundColor);
				previousBtn->enableText('C');
				previousBtn->setTextColor(buttonTextColor);
				previousBtn->setSelectedTextColor(buttonSelectedTextColor);
				previousBtn->setName("PREV");
				previousBtn->setText(prevButtonText);
				previousBtn->setPressDebounce(pressDebounce);
			}
			cout << "size with prev" << menuButtons.size() << endl;
			if(nextButton) {
				cout << "next button" << endl;
				nextBtn = new Button(nextButtonCenterX, scrollButtonCenterY, scrollButtonWidth, scrollButtonHeight);
				nextBtn->setSelectable();
				nextBtn->setBorder(buttonBorderColor, buttonBorderWidth);
				nextBtn->setSelectedBorder(buttonSelectedBorderColor, buttonSelectedBorderWidth);
				nextBtn->setCornerRadius(buttonCornerRadius);
				nextBtn->setBackgroundColor(buttonBackgroundColor);
				nextBtn->setSelectedBackgroundColor(buttonSelectedBackgroundColor);
				nextBtn->enableText('C');
				nextBtn->setTextColor(buttonTextColor);
				nextBtn->setSelectedTextColor(buttonSelectedTextColor);
				nextBtn->setName("NEXT");
				nextBtn->setText(nextButtonText);
				nextBtn->setPressDebounce(pressDebounce);
			}
			cout << "size with prev and next" << menuButtons.size() << endl;
		}
		// Enable scroll buttons
		if(scrollable && prevButton) previousBtn->touchEnable();
		if(scrollable && nextButton) nextBtn->touchEnable();
	}catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
	cfg->destroy();
}

/* Returns the pressed state of the first button which matches the provided name */
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
	bufferSaved = false;

}

/* Deselects the first button which matches the provided name */
void Menu::deselectButton(string name) {
	menuButtons[getVectorIndex(name)].deselect();
	bufferSaved = false;
}

/* Returns the index of the first item in the menu buttons vector which matches the provided name */
int Menu::getVectorIndex(string name) {
	int idx = 0;
	for(;idx<menuButtons.size();idx++) {
		if(name.compare(buttonNames[idx+topMenuItemIndex]) == 0) break;
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
	for(int idx = 0; idx<menuButtons.size(); idx++) {
		if(menuButtons[idx].isPressed()) {
			name.append(menuButtons[idx].getName());
			break;
		}
	}
	return name;
}